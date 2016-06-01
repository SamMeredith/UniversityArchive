#include "modmul.h"
#define max(a,b) \
  ({__typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

void myExp(mpz_t result, mpz_t x, mpz_t e, mpz_t N, int k);
void montExp(mpz_t result, mpz_t x, mpz_t e, mp_limb_t omega, mpz_t rho_sq,\
      mpz_t N, int k);
void montOmega (mp_limb_t*, mpz_t N);
void montRhoSq (mpz_t r, mpz_t N);

/*
Perform stage 1:

- read each 3-tuple of N, e and m from stdin,
- compute the RSA encryption c,
- then write the ciphertext c to stdout.
*/

void rsaEnc(mpz_t c, mpz_t m, mpz_t e, mpz_t N);

void stage1() {

  mpz_t N, e, message, result;

  mpz_inits(N,e,message,result,NULL);

  while (gmp_scanf( "%Zx", N) != EOF) {
    gmp_scanf( "%Zx", e );
    gmp_scanf( "%Zx", message );

    rsaEnc(result, message, e, N);

    gmp_printf( "%ZX\n", result );
  }

  mpz_clears(N, e, message, result, NULL);

}

void rsaEnc(mpz_t c, mpz_t m, mpz_t e, mpz_t N) {

  mp_limb_t omega = 0;
  mpz_t rho_sq;
  mpz_init(rho_sq);

  montOmega(&omega, N);
  montRhoSq(rho_sq, N);

  montExp(c, m, e, omega, rho_sq, N, 4);

  mpz_clear(rho_sq);
    
}
/*
Perform stage 2:

- read each 9-tuple of N, d, p, q, d_p, d_q, i_p, i_q and c from stdin,
- compute the RSA decryption m,
- then write the plaintext m to stdout.
*/
void crtDec(mpz_t m, mpz_t c, mpz_t d_p, mpz_t d_q, mpz_t p, mpz_t q,\
  mpz_t i_p, mpz_t i_q, mpz_t N);

void stage2() {

  mpz_t N, d, p, q, d_p, d_q, i_p, i_q, c, m;

  mpz_inits(N, d, p, q, d_p, d_q, i_p, i_q, c, m, NULL);

  while (gmp_scanf( "%Zx", N) != EOF) {
    gmp_scanf( "%Zx", d);
    gmp_scanf( "%Zx", p);
    gmp_scanf( "%Zx", q);
    gmp_scanf( "%Zx", d_p);
    gmp_scanf( "%Zx", d_q);
    gmp_scanf( "%Zx", i_p);
    gmp_scanf( "%Zx", i_q);
    gmp_scanf( "%Zx", c);

    crtDec(m, c, d_p, d_q, p, q, i_p, i_q, N);

    gmp_printf( "%ZX\n", m);
  }

  mpz_clears(N, d, p, q, d_p, d_q, i_p, i_q, c, m, NULL);
  
}

void crtDec(mpz_t m, mpz_t c, mpz_t d_p, mpz_t d_q, mpz_t p, mpz_t q,\
  mpz_t i_p, mpz_t i_q, mpz_t N) {

  mpz_t x_p, x_q, rho_sq;
  mp_limb_t omega = 0;
  
  mpz_inits(x_p, x_q, rho_sq, NULL);

  //simplify calculation using Lagrange's theorem
  //x_p = c^(d (mod p-1)) (mod p)
  //montMul requires operands < modulus, c need not be < p
  montOmega(&omega, p);
  montRhoSq(rho_sq, p);
  mpz_mod(m, c, p);
  montExp(x_p, m, d_p, omega, rho_sq, p, 4);
  //x_q = c^(d (mod q-1)) (mod q)

  montOmega(&omega, q);
  montRhoSq(rho_sq, q);
  mpz_mod(m, c, q);
  montExp(x_q, m, d_q, omega, rho_sq, q, 4);
 
  mpz_set_ui(m, 1);
  //By CRT
  //m = x_p*q*q^-1(mod p) + x_q*p*p^-1(mod q) (mod N);
  mpz_mul(m, q, i_q);
  mpz_mul(m, m, x_p);
  mpz_mul(x_q, x_q, p);
  mpz_addmul(m, x_q, i_p);
  mpz_mod(m, m, N);

  /*Garner's algorithm - write m = v_0 + v_1*p
    then v_0 = m mod p = x_p
    v_1 = q^-1(m - (m mod q) mod q) = (i_p * (x_q - x_p)) mod q 

    m = (((x_q - x_p) * (i_p)) mod q) * p + x_p  
  
  mpz_sub(m, x_q, x_p);
  mpz_mul(m, m, i_p);
  mpz_mod(m, m, q);
  mpz_mul(m, m, p);
  mpz_add(m, m, x_p);*/

  mpz_clears(x_p, x_q, rho_sq, NULL);

}

/*
Perform stage 3:

- read each 5-tuple of p, q, g, h and m from stdin,
- compute the ElGamal encryption c = (c_1,c_2),
- then write the ciphertext c to stdout.
*/
void ElGamalEnc(mpz_t m, mpz_t p, mpz_t q, mpz_t g, mpz_t h, mpz_t c1, mpz_t c2); 

void stage3() {

  mpz_t p, q, g, h, m, c1, c2;

  mpz_inits(p, q, g, h, m, c1, c2, NULL);

  while (gmp_scanf( "%Zx", p) != EOF) {
    gmp_scanf( "%Zx", q);
    gmp_scanf( "%Zx", g);
    gmp_scanf( "%Zx", h);
    gmp_scanf( "%Zx", m);

    ElGamalEnc(m, p, q, g, h, c1, c2);

    gmp_printf( "%ZX\n", c1);
    gmp_printf( "%ZX\n", c2); 
  }

  mpz_clears(p, q, g, h, m, c1, c2, NULL);
  
}

void ElGamalEnc(mpz_t m, mpz_t p, mpz_t q, mpz_t g, mpz_t h, mpz_t c1, mpz_t c2) {

  mpz_t r, rho_sq;
  gmp_randstate_t state;
  FILE* myRand;
  int buffsize = 2;
  mp_limb_t seedBuf[buffsize], omega = 0;

  mpz_inits(r, rho_sq, NULL);

  montOmega(&omega, p);
  montRhoSq(rho_sq, p);

  //read randomness r from /dev/urandom
  //sources suggest (will try to mention on marksheet) /urandom
  //is a suitable alternative to /random even in crypto applications
  //particularly as r is an ephemeral key, we don't need long term security
  //as it is discarded
  myRand = fopen("/dev/urandom", "r");
  if (myRand == NULL) {
    printf("Couldn't open urandom\n");
    abort();
  }
  size_t result = fread((unsigned long*)&seedBuf, sizeof(unsigned long), 2, myRand);
  if (result == 0) {
    printf("Failed to read from urandom\n");
    abort();
  }
  fclose(myRand);

  if (r->_mp_alloc < buffsize) {
    mpz_realloc2(r, mp_bits_per_limb*buffsize);
  }
  for (int i = 0; i < buffsize; i++) {
    r->_mp_d[i] = seedBuf[i];
    r->_mp_size++;
  }
  
  gmp_randinit_default(state);
  //seed DPRG with 128bits of randomness, more would not increase
  //security as textbook ElGamal is itself not CCA secure
  gmp_randseed(state, r);

  mpz_urandomm(r, state, p);
  
  montExp(c1, h, r, omega, rho_sq, p, 4);
  mpz_mul(c2, c1, m);
  mpz_mod(c2, c2, p);
  montExp(c1, g, r, omega, rho_sq, p, 4);

  mpz_clears(r, rho_sq, NULL);
  gmp_randclear(state);

}

/*
Perform stage 4:

- read each 5-tuple of p, q, g, x and c = (c_1,c_2) from stdin,
- compute the ElGamal decryption m,
- then write the plaintext m to stdout.
*/

void ElGamalDec(mpz_t m, mpz_t p, mpz_t q, mpz_t x, mpz_t c1, mpz_t c2); 

void stage4() {

  mpz_t N, p, q, g, x, m, c1, c2;

  mpz_inits(N, p, q, g, x, m, c1, c2, NULL);

  while (gmp_scanf( "%Zx", p) != EOF) {
    gmp_scanf( "%Zx", q);
    gmp_scanf( "%Zx", g);
    gmp_scanf( "%Zx", x);
    gmp_scanf( "%Zx", c1);
    gmp_scanf( "%Zx", c2);

    ElGamalDec(m, p, q, x, c1, c2);

    gmp_printf( "%ZX\n", m); 
  }

  mpz_clears(N, p, q, g, x, m, c1, c2, NULL);

}

void ElGamalDec(mpz_t m, mpz_t p, mpz_t q, mpz_t x, mpz_t c1, mpz_t c2) {

  mp_limb_t omega = 0;
  mpz_t rho_sq;
  mpz_init(rho_sq);

  montOmega(&omega, p);
  montRhoSq(rho_sq, p);

  mpz_neg(x, x);
  montExp(m, c1, x, omega, rho_sq, p, 4);
  mpz_mul(m, m, c2);
  mpz_mod(m, m, p);

  mpz_clear(rho_sq);
}

void montOmega (mp_limb_t* w, mpz_t N) {
 
  mpz_t r, w1;
  mpz_inits(r, w1, NULL);

  mpz_setbit(r, mp_bits_per_limb);
  mpz_invert(w1, N, r);
  mpz_sub(w1, r, w1);
  *w = w1->_mp_d[0];

  mpz_clears(r, w1, NULL);
 
}

void montRhoSq (mpz_t r, mpz_t N) {
  
  //calculate rho = b^k for smallest k s.t (2^64)^k > N
  //but this is just the mpz with 1 in limb #(modulus->_mp_size) + 1
  mpz_set_ui(r, 0);
  /*if (r->_mp_alloc < (2*N->_mp_size + 1)) {
    mpz_realloc2(r , mp_bits_per_limb*(2*N->_mp_size + 1));
  }
  r->_mp_d[2*N->_mp_size] = 1;
  r->_mp_size = 2*N->_mp_size + 1;*/
  mpz_setbit(r, 2*mp_bits_per_limb*N->_mp_size);
  mpz_mod(r, r, N);
}

void montMul (mpz_t result, mpz_t x, mpz_t y, mp_limb_t omega, mpz_t N) {

  mpz_t r;
  mpz_init(r);

  if (r->_mp_alloc < 2) {
    mpz_realloc2(r, mp_bits_per_limb*2);
  }
  for (int i = 0; i < (N->_mp_size); i++) {
    //could overflow, but thats fine as equivalent to working (mod base)
    r->_mp_d[r->_mp_alloc - 1] = ((mpz_getlimbn(r, 0) + mpz_getlimbn(y,i) *\
      mpz_getlimbn(x,0)) * omega);
    /*mpz_set_ui(u, ((mpz_getlimbn(r, 0) + mpz_getlimbn(y,i) *\
      mpz_getlimbn(x,0)) * mpz_getlimbn(omega,0)));*/
    mpz_addmul_ui(r, N, r->_mp_d[r->_mp_alloc - 1]);
    mpz_addmul_ui(r, x, mpz_getlimbn(y, i));
    mpz_tdiv_q_2exp(r, r, mp_bits_per_limb);
    //difference between this ^ and manually doing it is negligible
    /*for (int j = 0; j < (r->_mp_size - 1); j++) {
      r->_mp_d[j] = r->_mp_d[j+1];
    }
    r->_mp_size--;*/
  }
  if (mpz_cmp(r, N) >= 0) {
    mpz_sub(r, r, N);
  }
  mpz_set(result, r);

  mpz_clear(r);

}

void montExp(mpz_t result, mpz_t x, mpz_t e, mp_limb_t omega, mpz_t rho_sq,\
      mpz_t N, int k) {

  int j, i, l, u, invert = 0;
  mpz_t T[(2 << (k-2))];
  mpz_t x_sq, x_mont, mpz_one;

  //initialise variables
  for (j = 0; j < (2 << (k-2)) ; j++) {
    mpz_init(T[j]);
  }
  mpz_inits(x_sq, x_mont, mpz_one, NULL);

  //deal with negative exponents (i.e in ElGamal)
  if (mpz_sgn(e) == -1) {
    invert = 1;
    mpz_abs(e, e);
  }

  mpz_set_ui(mpz_one, 1);

  //precompute values (only odd powers)
  montMul(x_mont, x, rho_sq, omega, N);
  mpz_set(T[0], x_mont);
  montMul(x_sq, x_mont, x_mont, omega, N);
  for (j = 1; j < (2 << (k-2)); j++) {
    montMul(T[j], T[j-1], x_sq, omega, N);
  }

  montMul(result, mpz_one, rho_sq, omega, N); 
  i = (int)mpz_sizeinbase(e, 2) - 1;

  while (i >= 0) {
    u = 0;
    //i'th bit is 0, window size 0
    if (!mpz_tstbit(e, i)) {
      l = i;
    }
    //i'th bit is 1, throw window out (up to) k
    else {
      l = max(i-k+1, 0);
      //reduce window until 1 found
      while (!mpz_tstbit(e, l)) {
        l++;
      }
      //calculate size within window
      for (j = l; j <= i; j++) {
        if (mpz_tstbit(e,j)) {
          u += (j == l) ? 1 : (2 << (j-l-1));
        }
      }
    }
    //double result window-size times
    for (j = 0; j < (i-l+1); j++) {
      montMul(result, result, result, omega, N);
    }
    //multiply by additional powers of x if needed
    if (u != 0) {
      montMul(result, result, T[(u-1)/2], omega, N);
    }

    i = l-1;
  }

  montMul(result, result, mpz_one, omega, N);

  if (invert) mpz_invert(result, result, N);

  //free
  for (j = 0; j < (2 << (k-2)) ; j++) {
    mpz_clear(T[j]);
  }
  mpz_clears(x_sq, x_mont, mpz_one, NULL);
}

//2k-ary slide exponentiation, without Montgomery multiplication
void myExp(mpz_t result, mpz_t x, mpz_t e, mpz_t N, int k) {

  int j, i, l, u, invert = 0;
  mpz_t T[(2 << (k-2))];
  mpz_t x_sq;

  for (j = 0; j < (2 << (k-2)) ; j++) {
    mpz_init(T[j]);
  }
  mpz_init(x_sq);

  if (mpz_sgn(e) == -1) {
    invert = 1;
    mpz_abs(e, e);
  }

  mpz_set(T[0], x);
  mpz_mul(x_sq, x, x);
  for (j = 1; j < (2 << (k-2)); j++) {
    mpz_mul(T[j], T[j-1], x_sq);
    mpz_mod(T[j], T[j], N);
  }

  mpz_set_ui(result, 1); 
  i = (int)mpz_sizeinbase(e, 2) - 1;

  while (i >= 0) {
    u = 0;
    if (!mpz_tstbit(e, i)) {
      l = i;
    }
    else {
      l = max(i-k+1, 0);
      while (!mpz_tstbit(e, l)) {
        l++;
      }
      for (j = l; j <= i; j++) {
        if (mpz_tstbit(e,j)) {
          u += (j == l) ? 1 : (2 << (j-l-1));
        }
      }
    }
    for (j = 0; j < (i-l+1); j++) {
      mpz_mul(result, result, result);
      mpz_mod(result, result, N);
    }
    if (u != 0) {
      mpz_mul(result, result, T[(u-1)/2]);
      mpz_mod(result, result, N);
    }

    i = l-1;
  }

  if (invert) mpz_invert(result, result, N);

  for (j = 0; j < (2 << (k-2)) ; j++) {
    mpz_clear(T[j]);
  }
  mpz_clear(x_sq);
}

void test() {
  
  mpz_t rho, omega, p, i_p, q, i_q, e, k, d, d_p, d_q, g, h, m, c1, c2, gcd,\
    result, p_sub, q_sub, N, phi_N;
  gmp_randstate_t state;
  int msec = 0;
  clock_t start, diff;

  mpz_inits(rho, omega, p, i_p, q, i_q, N, e, k, d, d_p, d_q, g, h, m, c1, c2,\
    gcd, result, phi_N, p_sub, q_sub, NULL);
  gmp_randinit_default(state);  

  for (int counter = 0; counter < 100; counter++) {

    /*Generate random primes p,q*/
    while (!mpz_probab_prime_p(p,25)) {
      mpz_urandomb(p, state, 1024);
    }
    while (!mpz_probab_prime_p(q,25)) {
      mpz_urandomb(q, state, 1024);
    }
    //Calculate inv(p) (mod q) and inv(q) (mod p) for CRT
    mpz_invert(i_p, p, q);
    mpz_invert(i_q, q, p);
    //Calculate N (modulus) and phi(N)
    mpz_mul(N, q, p);
    mpz_sub_ui(p_sub, p, 1);
    mpz_sub_ui(q_sub, q, 1);
    mpz_mul(phi_N, p_sub, q_sub);

    //generate random exponent until gcd(e, phi_N) = 1
    mpz_urandomb(e, state, 64);
    mpz_gcd(gcd, e, phi_N);
    while (mpz_cmp_ui(gcd, 1) && mpz_cmp_ui(e, 0)) {
      mpz_urandomb(e, state, 64);
      mpz_gcd(gcd, e, phi_N);
    }

    //calculate d = inv(e) mod phi_N
    mpz_invert(d, e, phi_N);
    //calculate d (mod p-1) and d (mod q-1) for CRT
    mpz_mod(d_p, d, p_sub);
    mpz_mod(d_q, d, q_sub);

    //generate random message
    mpz_urandomb(m, state, 64);
    
    start = clock();

    //encrypt message
    rsaEnc(c1, m, e, N);

    //decrypt ciphertext
    //rsaDec(result, c1, d, N);
    crtDec(result, c1, d_p, d_q, p, q, i_p, i_q, N); 

    diff = clock() - start;
    msec += diff;
    

    //check decryption matches ciphertext
    if (mpz_cmp(result,m)) {
      printf("rsa fails\n");
      gmp_printf("e = %Zd\n", e);
      gmp_printf("d = %Zd\n", d);
      gmp_printf("N = %Zd\n", N);
      gmp_printf("m = %Zd\n", m);
      gmp_printf("r = %Zd\n", result);
      gmp_randclear(state);
      mpz_clears(rho, omega, p, i_p, q, i_q, N, e, k, d, d_p, d_q, g, h, m, c1, c2,\
        gcd, result, phi_N, p_sub, q_sub, NULL);
      abort();
    }
    
    mpz_set_ui(p, 0);
    mpz_set_ui(q, 0);
    //generate random prime q, 256 bits, until kq+1 also prime
    while (!mpz_probab_prime_p(p,25)) {
      while (!mpz_probab_prime_p(q,25)) {
        mpz_urandomb(q, state, 256);
      }
      mpz_urandomb(k, state, 1024);
      mpz_mul(p, q, k);
      mpz_add_ui(p, p, 1);
    }

    //calculate generator
    mpz_urandomm(g, state, p);
    mpz_powm(h, g, k, p);
    while(!mpz_cmp_ui(h, 1)) {
      mpz_urandomm(g, state, p);
      mpz_powm(h, g, k, p);
    }
    
    //calculate public key (just use e as secret key) 
    mpz_powm(h, g, e, p);

    //encrypt message
    ElGamalEnc(m, p, q, g, h, c1, c2);

    //decrypt ciphertext
    ElGamalDec(result, p, q, e, c1, c2);

    //check decryption matches ciphertext
    if (mpz_cmp(result,m)) {
      printf("ElGamal fails");
      gmp_printf("e = %Zd\n", e);
      gmp_printf("d = %Zd\n", d);
      gmp_printf("N = %Zd\n", N);
      gmp_printf("m = %Zd\n", m);
      gmp_printf("r = %Zd\n", result);
      gmp_randclear(state);
      mpz_clears(rho, omega, p, i_p, q, i_q, N, e, k, d, d_p, d_q, g, h, m, c1, c2,\
        gcd, result, phi_N, p_sub, q_sub, NULL);
      abort();
    }
  }
  
  gmp_randclear(state);
  mpz_clears(rho, omega, p, i_p, q, i_q, N, e, k, d, d_p, d_q, g, h, m, c1, c2,\
    gcd, result, phi_N, p_sub, q_sub, NULL);

  msec = msec * 1000 / CLOCKS_PER_SEC;
  printf("Time taken %d seconds %d milliseconds\n", msec/1000, msec%1000);
}

/*
The main function acts as a driver for the assignment by simply invoking
the correct function for the requested stage.
*/

int main( int argc, char* argv[] ) {
  if( argc != 2 ) {
    abort();
  }

  if     ( !strcmp( argv[ 1 ], "stage1" ) ) {
    stage1();
  }
  else if( !strcmp( argv[ 1 ], "stage2" ) ) {
    stage2();
  }
  else if( !strcmp( argv[ 1 ], "stage3" ) ) {
    stage3();
  }
  else if( !strcmp( argv[ 1 ], "stage4" ) ) {
    stage4();
  }
  /*else if( !strcmp( argv[ 1 ], "test" ) ) {
    test();
  }*/
  else {
    abort();
  }

  return 0;
}
