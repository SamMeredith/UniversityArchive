/*High-performance computing coursework. Single threaded fluid dynamics algorithm provided.
  My contribution primarily functions within 'timestep'.
  HPC-MPI uses message passing (physically local distributed communication) and master-
  slave multi-threading to speed up algorithm*/

/*
** Code to implement a d2q9-bgk lattice boltzmann scheme.
** 'd2' inidates a 2-dimensional grid, and
** 'q9' indicates 9 velocities per grid cell.
** 'bgk' refers to the Bhatnagar-Gross-Krook collision step.
**
** The 'speeds' in each cell are numbered as follows:
**
** 6 2 5
**  \|/
** 3-0-1
**  /|\
** 7 4 8
**
** A 2D grid:
**
**           cols
**       --- --- ---
**      | D | E | F |
** rows  --- --- ---
**      | A | B | C |
**       --- --- ---
**
** 'unwrapped' in row major order to give a 1D array:
**
**  --- --- --- --- --- ---
** | A | B | C | D | E | F |
**  --- --- --- --- --- ---
**
** Grid indicies are:
**
**          ny
**          ^       cols(jj)
**          |  ----- ----- -----
**          | | ... | ... | etc |
**          |  ----- ----- -----
** rows(ii) | | 1,0 | 1,1 | 1,2 |
**          |  ----- ----- -----
**          | | 0,0 | 0,1 | 0,2 |
**          |  ----- ----- -----
**          ----------------------> nx
**
** Note the names of the input parameter and obstacle files
** are passed on the command line, e.g.:
**
**   d2q9-bgk.exe input.params obstacles.dat
**
** Be sure to adjust the grid dimensions in the parameter file
** if you choose a different obstacle file.
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<mpi.h>

#define NSPEEDS         9
#define FINALSTATEFILE  "final_state.dat"
#define AVVELSFILE      "av_vels.dat"

/* struct to hold the parameter values */
typedef struct {
  int    nx;            /* no. of cells in x-direction */
  int    ny;            /* no. of cells in y-direction */
  int    maxIters;      /* no. of iterations */
  int    reynolds_dim;  /* dimension for Reynolds number */
  double density;       /* density per link */
  double accel;         /* density redistribution */
  double omega;         /* relaxation parameter */
} t_param;

enum boolean { FALSE, TRUE };

/*
** function prototypes
*/

/* load params, allocate memory, load obstacles & initialise fluid particle densities */
int initialise(const char* paramfile, const char* obstaclefile,
        t_param* params, float** cells_ptr, float** tmp_cells_ptr, 
         int** obstacles_ptr, double** av_vels_ptr);

/* 
** The main calculation methods.
** timestep calls, in order, the functions:
** accelerate_flow(), propagate(), rebound() & collision()
*/
int timestep(const t_param params, float* cells, float* tmp_cells, int* obstacles);
int accelerate_flow(const t_param params, float* cells, int* obstacles);
int propagate(const t_param params, float* cells, float* tmp_cells);
int collision(const t_param params, float* cells, float* tmp_cells, int* obstacles);
int write_values(const t_param params, float* cells, int* obstacles, double* av_vels);

/* finalise, including freeing up allocated memory */
int finalise(const t_param* params, float** cells_ptr, float** tmp_cells_ptr,
       int** obstacles_ptr, double** av_vels_ptr);

/* Sum all the densities in the grid.
** The total should remain constant from one timestep to the next. */
double total_density(const t_param params, float* cells);

/* compute average velocity */
double av_velocity(const t_param params, float* cells, int* obstacles);

/* calculate Reynolds number */
double calc_reynolds(const t_param params, float* cells, int* obstacles);

/* utility functions */
int local_start_calc(int numberOfRows, int size, int rank);
void die(const char* message, const int line, const char *file);
void usage(const char* exe);

int local_start;
int local_end;

/*
** main program:
** initialise, timestep loop, finalise
*/
int main(int argc, char* argv[])
{
  char*    paramfile = NULL;    /* name of the input parameter file */
  char*    obstaclefile = NULL; /* name of a the input obstacle file */
  t_param  params;              /* struct to hold parameter values */
  float* cells     = NULL;    /* grid containing fluid densities */
  float* tmp_cells = NULL;    /* scratch space */
  int*     obstacles = NULL;    /* grid indicating which cells are blocked */
  double*  av_vels   = NULL;    /* a record of the av. velocity computed for each timestep */
  int      ii;                  /* generic counter */
  struct timeval timstr;        /* structure to hold elapsed time */
  struct rusage ru;             /* structure to hold CPU time--system and user */
  double tic,toc;               /* floating point numbers to calculate elapsed wallclock time */
  double usrtim;                /* floating point number to record elapsed user CPU time */
  double systim;                /* floating point number to record elapsed system CPU time */
  int flag,rank,size;
  int numberOfRows,source,sourceStart,sourceEnd;
  MPI_Status status;
  double reynolds;

  /* parse the command line */
  if(argc != 3) {
    usage(argv[0]);
  }
  else{
    paramfile = argv[1];
    obstaclefile = argv[2];
  }

  /*Initialise*/
  MPI_Init( &argc, &argv );
  /*Test initialisation was sucessful*/
  MPI_Initialized(&flag);
  if (!flag) {
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }

  /* initialise our data structures and load values from file */
  initialise(paramfile, obstaclefile, &params, &cells, &tmp_cells, &obstacles, &av_vels);

  /*Find rank and size*/
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  /*calculate start and end points for each rank*/
  local_start = local_start_calc(params.ny,size,rank);
  local_end = local_start_calc(params.ny,size,rank+1);

  /* iterate for maxIters timesteps */
  gettimeofday(&timstr,NULL);
  tic=timstr.tv_sec+(timstr.tv_usec/1000000.0);

  for (ii=0;ii<params.maxIters;ii++) {
    timestep(params,cells,tmp_cells,obstacles);
    av_vels[ii] = av_velocity(params,cells,obstacles);
#ifdef DEBUG
    printf("==timestep: %d==\n",ii);
    printf("av velocity: %.12E\n", av_vels[ii]);
    printf("tot density: %.12E\n",total_density(params,cells));
#endif
  }
  gettimeofday(&timstr,NULL);
  toc=timstr.tv_sec+(timstr.tv_usec/1000000.0);
  getrusage(RUSAGE_SELF, &ru);
  timstr=ru.ru_utime;        
  usrtim=timstr.tv_sec+(timstr.tv_usec/1000000.0);
  timstr=ru.ru_stime;        
  systim=timstr.tv_sec+(timstr.tv_usec/1000000.0);

  /*each process has carried out calculations for its own section 
    need to collate the information in master process*/
  if (rank != 0) {
    numberOfRows = local_end-local_start;
    /*send updated region*/
    MPI_Send(&cells[local_start*params.nx*9],9*params.nx*numberOfRows,MPI_FLOAT,0,0,
      MPI_COMM_WORLD);
  }
  else {
    /*recieve from all ranks*/
    for (source = 1; source < size; source++) {
      sourceStart = local_start_calc(params.ny,size,source);
      sourceEnd = local_start_calc(params.ny,size,source+1);
      numberOfRows = sourceEnd - sourceStart;
      MPI_Recv(&cells[sourceStart*params.nx*9],9*params.nx*numberOfRows,
        MPI_FLOAT,source,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    }
  }

  reynolds = calc_reynolds(params,cells,obstacles);

  if (rank ==0) {
    /* write final values and free memory */
    printf("==done==\n");
    printf("Reynolds number:\t\t%.12E\n",reynolds);
    printf("Elapsed time:\t\t\t%.6lf (s)\n", toc-tic);
    printf("Elapsed user CPU time:\t\t%.6lf (s)\n", usrtim);
    printf("Elapsed system CPU time:\t%.6lf (s)\n", systim);
    write_values(params,cells,obstacles,av_vels); /*<- needs parallelising*/
  }

  finalise(&params, &cells, &tmp_cells, &obstacles, &av_vels);
  
  MPI_Finalize();
  return EXIT_SUCCESS;
}

int timestep(const t_param params, float* cells, float* tmp_cells, int* obstacles)
{
  accelerate_flow(params,cells,obstacles);
  propagate(params,cells,tmp_cells);
  collision(params,cells,tmp_cells,obstacles);
  return EXIT_SUCCESS; 
}

int accelerate_flow(const t_param params, float* cells, int* obstacles)
{
  int ii,jj,pos;     /* generic counters */
  double w1,w2;  /* weighting factors */
  int rank,size;

  /* compute weighting factors */
  w1 = params.density * params.accel / 9.0;
  w2 = params.density * params.accel / 36.0;

   /*Find rank and size*/
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  /* modify the 2nd row of the grid */
  ii=params.ny - 2;
  /*only one process needs to work here*/
  if ((local_start <= ii) && (ii < local_end)) {
    for(jj=0;jj<params.nx;jj++) {
      pos = (ii*params.nx + jj)*9;
      /* if the cell is not occupied and
      ** we don't send a density negative */
      if( !obstacles[ii*params.nx + jj] && 
        (cells[pos+3] - w1) > 0.0 &&
        (cells[pos+6] - w2) > 0.0 &&
        (cells[pos+7] - w2) > 0.0 ) {
        /* increase 'east-side' densities */
        cells[pos+1] += w1;
        cells[pos+5] += w2;
        cells[pos+8] += w2;
        /* decrease 'west-side' densities */
        cells[pos+3] -= w1;
        cells[pos+6] -= w2;
        cells[pos+7] -= w2;
      }
    } 
  }

  return EXIT_SUCCESS;
}

int propagate(const t_param params, float* cells, float* tmp_cells)
{
  int ii,jj;            /* generic counters */
  int x_e,x_w,y_n,y_s;  /* indices of neighbouring cells */
  int pos; /*index of current cell */
  int rank,size;
  int upper,lower;
  int upperHalo, upperRecv, lowerHalo, lowerRecv;

  MPI_Status status;

  /*Find rank and size*/
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);
 
  /*work out processes to communicate with*/
  upper = (rank + 1) % size;
  lower = (rank == 0) ? (rank + size - 1) : (rank - 1);

  /*Send-recieve halos*/
  /*Send upper halo*/
  upperHalo = local_end - 1;
  lowerRecv = (rank != 0) ? (local_start-1) : params.ny -1;
  MPI_Sendrecv(&cells[upperHalo*params.nx*9],params.nx*9,MPI_FLOAT,upper,0,
    &cells[lowerRecv*params.nx*9],params.nx*9,MPI_FLOAT,lower,0,MPI_COMM_WORLD,&status);
    
  /*send lower halo*/
  lowerHalo = local_start;
  upperRecv = local_start_calc(params.ny,size,upper);
  MPI_Sendrecv(&cells[lowerHalo*params.nx*9],params.nx*9,MPI_FLOAT,lower,0,
    &cells[upperRecv*params.nx*9],params.nx*9,MPI_FLOAT,upper,0,MPI_COMM_WORLD,&status);

  /* loop over relevant cells */
  for(ii=local_start;ii<local_end;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      pos = (ii*params.nx + jj)*9;
      /* determine indices of axis-direction neighbossurs
      ** respecting periodic boundary conditions (wrap around) */
      y_n = (ii + 1) % params.ny;
      x_e = (jj + 1) % params.nx;
      y_s = (ii == 0) ? (ii + params.ny - 1) : (ii - 1);
      x_w = (jj == 0) ? (jj + params.nx - 1) : (jj - 1);
      /* propagate densities to neighbouring cells, following
      ** appropriate directions of travel and writing into
      ** scratch space grid */
      tmp_cells[pos+0] = cells[pos+0]; /*no movement */
      tmp_cells[pos+1] = cells[(ii*params.nx + x_w)*9+1]; /*west*/
      tmp_cells[pos+2] = cells[(y_s*params.nx + jj)*9+2]; /*south*/
      tmp_cells[pos+3] = cells[(ii*params.nx + x_e)*9+3]; /*east*/
      tmp_cells[pos+4] = cells[(y_n*params.nx + jj)*9+4]; /*north*/
      tmp_cells[pos+5] = cells[(y_s*params.nx + x_w)*9+5]; /*south-west*/
      tmp_cells[pos+6] = cells[(y_s*params.nx + x_e)*9+6]; /*south-east*/
      tmp_cells[pos+7] = cells[(y_n*params.nx + x_e)*9+7]; /*north-east*/
      tmp_cells[pos+8] = cells[(y_n*params.nx + x_w)*9+8]; /*north-west*/    
    }
  }

  return EXIT_SUCCESS;
}


int collision(const t_param params, float* cells, float* tmp_cells, int* obstacles)
{
  int ii,jj,kk,pos;                 /* generic counters */
  const double w0 = 4.0/9.0;    /* weighting factor */
  const double w1 = 1.0/9.0;    /* weighting factor */
  const double w2 = 1.0/36.0;   /* weighting factor */
  double u_x,u_y;               /* av. velocities in x and y directions */
  double u[NSPEEDS];            /* directional velocities */
  double d_equ[NSPEEDS];        /* equilibrium densities */
  double u_sq;                  /* squared velocity */
  double local_density;         /* sum of densities in a particular cell */
  double calc1;
  int rank,size;

  /*At this point, propagate has updated tmp_cells, rebound has updated blocked
    cells in cells array. Now want to split the work of collision between
    processes*/

  /*Find rank and size*/
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  /* loop over the cells in the grid
  ** NB the collision step is called after
  ** the propagate step and so values of interest
  ** are in the scratch-space grid */
  for(ii=local_start;ii<local_end;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      pos = (ii*params.nx + jj)*9;
      if(obstacles[ii*params.nx + jj]) {
        /* called after propagate, so taking values from scratch space
        ** mirroring, and writing into main grid */
        cells[pos+1] = tmp_cells[pos+3];
        cells[pos+2] = tmp_cells[pos+4];
        cells[pos+3] = tmp_cells[pos+1];
        cells[pos+4] = tmp_cells[pos+2];
        cells[pos+5] = tmp_cells[pos+7];
        cells[pos+6] = tmp_cells[pos+8];
        cells[pos+7] = tmp_cells[pos+5];
        cells[pos+8] = tmp_cells[pos+6];
      }
      /* don't consider occupied cells */
      else {
        /* compute local density total */
        local_density = 0.0;
        for(kk=0;kk<NSPEEDS;kk++) {
          local_density += tmp_cells[pos+kk];
        }
        /* compute x velocity component */
        u_x = (tmp_cells[pos+1] + tmp_cells[pos+5] + 
          tmp_cells[pos+8] - (tmp_cells[pos+3] + 
          tmp_cells[pos+6] + tmp_cells[pos+7])) / local_density;
        /* compute y velocity component */
        u_y = (tmp_cells[pos+2] + tmp_cells[pos+5] + 
          tmp_cells[pos+6] - (tmp_cells[pos+4] + 
          tmp_cells[pos+7] + tmp_cells[pos+8])) / local_density;
        /* velocity squared */ 
        u_sq = u_x * u_x + u_y * u_y;
        /* directional velocity components */
        u[1] =   u_x;        /* east */
        u[2] =         u_y;  /* north */
        u[3] = - u_x;        /* west */
        u[4] =       - u_y;  /* south */
        u[5] =   u_x + u_y;  /* north-east */
        u[6] = - u_x + u_y;  /* north-west */
        u[7] = - u_x - u_y;  /* south-west */
        u[8] =   u_x - u_y;  /* south-east */
        /* equilibrium densities */
        calc1 = u_sq * 1.5;
        /* zero velocity density: weight w0 */
        d_equ[0] = w0 * local_density * (1.0 - u_sq * 1.5);
        /* axis speeds: weight w1 */
        d_equ[1] = w1 * local_density * (1.0 + u[1] * 3
           + (u[1] * u[1]) * 4.5 - calc1);
        d_equ[2] = w1 * local_density * (1.0 + u[2] * 3
           + (u[2] * u[2]) * 4.5 - calc1);
        d_equ[3] = w1 * local_density * (1.0 + u[3] * 3
           + (u[3] * u[3]) * 4.5 - calc1);
        d_equ[4] = w1 * local_density * (1.0 + u[4] * 3
           + (u[4] * u[4]) * 4.5 - calc1);
        /* diagonal speeds: weight w2 */
        d_equ[5] = w2 * local_density * (1.0 + u[5] * 3
           + (u[5] * u[5]) * 4.5 - calc1);
        d_equ[6] = w2 * local_density * (1.0 + u[6] * 3
           + (u[6] * u[6]) * 4.5 - calc1);
        d_equ[7] = w2 * local_density * (1.0 + u[7] * 3
           + (u[7] * u[7]) * 4.5 - calc1);
        d_equ[8] = w2 * local_density * (1.0 + u[8] * 3
           + (u[8] * u[8]) * 4.5 - calc1);
        /* relaxation step */
        for(kk=0;kk<NSPEEDS;kk++) {
          cells[pos+kk] = (tmp_cells[pos+kk]
             + params.omega * 
             (d_equ[kk] - tmp_cells[pos+kk]));
        }
      }
    }
  }

  return EXIT_SUCCESS; 
}

int initialise(const char* paramfile, const char* obstaclefile,
         t_param* params, float** cells_ptr, float** tmp_cells_ptr, 
         int** obstacles_ptr, double** av_vels_ptr)
{
  char   message[1024];  /* message buffer */
  FILE   *fp;            /* file pointer */
  int    ii,jj;       /* generic counters */
  int    xx,yy;          /* generic array indices */
  int    blocked;        /* indicates whether a cell is blocked by an obstacle */ 
  int    retval;         /* to hold return value for checking */
  double w0,w1,w2;       /* weighting factors */

  /* open the parameter file */
  fp = fopen(paramfile,"r");
  if (fp == NULL) {
    sprintf(message,"could not open input parameter file: %s", paramfile);
    die(message,__LINE__,__FILE__);
  }

  /* read in the parameter values */
  retval = fscanf(fp,"%d\n",&(params->nx));
  if(retval != 1) die ("could not read param file: nx",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->ny));
  if(retval != 1) die ("could not read param file: ny",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->maxIters));
  if(retval != 1) die ("could not read param file: maxIters",__LINE__,__FILE__);
  retval = fscanf(fp,"%d\n",&(params->reynolds_dim));
  if(retval != 1) die ("could not read param file: reynolds_dim",__LINE__,__FILE__);
  retval = fscanf(fp,"%lf\n",&(params->density));
  if(retval != 1) die ("could not read param file: density",__LINE__,__FILE__);
  retval = fscanf(fp,"%lf\n",&(params->accel));
  if(retval != 1) die ("could not read param file: accel",__LINE__,__FILE__);
  retval = fscanf(fp,"%lf\n",&(params->omega));
  if(retval != 1) die ("could not read param file: omega",__LINE__,__FILE__);

  /* and close up the file */
  fclose(fp);

  /* 
  ** Allocate memory.
  **
  ** Remember C is pass-by-value, so we need to
  ** pass pointers into the initialise function.
  **
  ** NB we are allocating a 1D array, so that the
  ** memory will be contiguous.  We still want to
  ** index this memory as if it were a (row major
  ** ordered) 2D array, however.  We will perform
  ** some arithmetic using the row and column
  ** coordinates, inside the square brackets, when
  ** we want to access elements of this array.
  **
  ** Note also that we are using a structure to
  ** hold an array of 'speeds'.  We will allocate
  ** a 1D array of these structs.
  */

  /* main grid */
  *cells_ptr = (float*)malloc(sizeof(float)*(params->ny*params->nx)*NSPEEDS);
  if (*cells_ptr == NULL) 
    die("cannot allocate memory for cells",__LINE__,__FILE__);

  /* 'helper' grid, used as scratch space */
  *tmp_cells_ptr = (float*)malloc(sizeof(float)*(params->ny*params->nx)*NSPEEDS);
  if (*tmp_cells_ptr == NULL) 
    die("cannot allocate memory for tmp_cells",__LINE__,__FILE__);
  
  /* the map of obstacles */
  *obstacles_ptr = (int*)malloc(sizeof(int)*(params->ny*params->nx));
  if (*obstacles_ptr == NULL) 
    die("cannot allocate column memory for obstacles",__LINE__,__FILE__);

  /* initialise densities */
  w0 = params->density * 4.0/9.0;
  w1 = params->density      /9.0;
  w2 = params->density      /36.0;

  for(ii=0;ii<params->ny;ii++) {
    for(jj=0;jj<params->nx;jj++) {
      /* centre */
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS] = w0;
      /* axis directions */
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+1] = w1;
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+2] = w1;
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+3] = w1;
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+4] = w1;
      /* diagonals */
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+5] = w2;
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+6] = w2;
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+7] = w2;
      (*cells_ptr)[(ii*params->nx + jj)*NSPEEDS+8] = w2;
    }
  }

  /* first set all cells in obstacle array to zero */ 
  for(ii=0;ii<params->ny;ii++) {
    for(jj=0;jj<params->nx;jj++) {
      (*obstacles_ptr)[ii*params->nx + jj] = 0;
    }
  }

  /* open the obstacle data file */
  fp = fopen(obstaclefile,"r");
  if (fp == NULL) {
    sprintf(message,"could not open input obstacles file: %s", obstaclefile);
    die(message,__LINE__,__FILE__);
  }

  /* read-in the blocked cells list */
  while( (retval = fscanf(fp,"%d %d %d\n", &xx, &yy, &blocked)) != EOF) {
    /* some checks */
    if ( retval != 3)
      die("expected 3 values per line in obstacle file",__LINE__,__FILE__);
    if ( xx<0 || xx>params->nx-1 )
      die("obstacle x-coord out of range",__LINE__,__FILE__);
    if ( yy<0 || yy>params->ny-1 )
      die("obstacle y-coord out of range",__LINE__,__FILE__);
    if ( blocked != 1 ) 
      die("obstacle blocked value should be 1",__LINE__,__FILE__);
    /* assign to array */
    (*obstacles_ptr)[yy*params->nx + xx] = blocked;
  }
  
  /* and close the file */
  fclose(fp);

  /* 
  ** allocate space to hold a record of the avarage velocities computed 
  ** at each timestep
  */
  *av_vels_ptr = (double*)malloc(sizeof(double)*params->maxIters);

  return EXIT_SUCCESS;
}

int finalise(const t_param* params, float** cells_ptr, float** tmp_cells_ptr,
       int** obstacles_ptr, double** av_vels_ptr)
{
  /* 
  ** free up allocated memory
  */
  free(*cells_ptr);
  *cells_ptr = NULL;

  free(*tmp_cells_ptr);
  *tmp_cells_ptr = NULL;

  free(*obstacles_ptr);
  *obstacles_ptr = NULL;

  free(*av_vels_ptr);
  *av_vels_ptr = NULL;

  return EXIT_SUCCESS;
}

double av_velocity(const t_param params, float* cells, int* obstacles)
{
  int    ii,jj,kk,pos;       /* generic counters */
  int    tot_cells = 0;  /* no. of cells used in calculation */
  double local_density;  /* total density in cell */
  double u_x;            /* x-component of velocity for current cell */
  double u_y;            /* y-component of velocity for current cell */
  double tot_u;          /* accumulated magnitudes of velocity for each cell */
  double global_tot_u;
  int global_tot_cells = 0;
  int rank,size;

  /* initialise */
  tot_u = 0.0;
  global_tot_u = 0.0;

  /*Find rank and size*/
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  /* loop over all non-blocked cells */
  for(ii=local_start;ii<local_end;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      pos = (ii*params.nx + jj)*9;
      /* ignore occupied cells */
      if(!obstacles[ii*params.nx + jj]) {
        /* local density total */
        local_density = 0.0;
        for(kk=0;kk<NSPEEDS;kk++) {
          local_density += cells[pos+kk];
        }
        /* compute x velocity component */
        u_x = (cells[pos+1] + cells[pos+5] + cells[pos+8]
          - (cells[pos+3] + cells[pos+6] + cells[pos+7]))
          / local_density;
        /* compute y velocity component */
        u_y = (cells[pos+2] + cells[pos+5] + cells[pos+6]
          - (cells[pos+4] + cells[pos+7] + cells[pos+8]))
          / local_density;
        /* accumulate the norm of x- and y- velocity components */
        tot_u += sqrt((u_x * u_x) + (u_y * u_y));
        /* increase counter of inspected cells */
        ++tot_cells;
      }
    }
  }
  /*collect individual sums in master process*/
  MPI_Reduce(&tot_u,&global_tot_u,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(&tot_cells,&global_tot_cells,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
  /*copy sums back over*/
  if (rank == 0) {
    tot_u = global_tot_u;
    tot_cells = global_tot_cells;
  }

  return tot_u / (double)tot_cells;
}

double calc_reynolds(const t_param params, float* cells, int* obstacles)
{
  const double viscosity = 1.0 / 6.0 * (2.0 / params.omega - 1.0);
  
  return av_velocity(params,cells,obstacles) * params.reynolds_dim / viscosity;
}

double total_density(const t_param params, float* cells)
{
  int ii,jj,kk;        /* generic counters */
  double total = 0.0;  /* accumulator */

  for(ii=0;ii<params.ny;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      for(kk=0;kk<NSPEEDS;kk++) {
  total += cells[(ii*params.nx + jj)*9+kk];
      }
    }
  }
  
  return total;
}

int write_values(const t_param params, float* cells, int* obstacles, double* av_vels)
{
  FILE* fp;                     /* file pointer */
  int ii,jj,kk, pos;            /* generic counters */
  const double c_sq = 1.0/3.0;  /* sq. of speed of sound */
  double local_density;         /* per grid cell sum of densities */
  double pressure;              /* fluid pressure in grid cell */
  double u_x;                   /* x-component of velocity in grid cell */
  double u_y;                   /* y-component of velocity in grid cell */
  double u;                     /* norm--root of summed squares--of u_x and u_y */

  fp = fopen(FINALSTATEFILE,"w");
  if (fp == NULL) {
    die("could not open file output file",__LINE__,__FILE__);
  }

  for(ii=0;ii<params.ny;ii++) {
    for(jj=0;jj<params.nx;jj++) {
      pos = (ii*params.nx + jj)*9;
      /* an occupied cell */
      if(obstacles[ii*params.nx + jj]) {
        u_x = u_y = u = 0.0;
        pressure = params.density * c_sq;
      }
      /* no obstacle */
      else {
        local_density = 0.0;
        for(kk=0;kk<NSPEEDS;kk++) {
          local_density += cells[pos+kk];
        }
        /* compute x velocity component */
        u_x = (cells[pos+1] + cells[pos+5] + cells[pos+8]
          - (cells[pos+3] + cells[pos+6] + cells[pos+7]))
          / local_density;
        /* compute y velocity component */
        u_y = (cells[pos+2] + cells[pos+5] + cells[pos+6]
          - (cells[pos+4] + cells[pos+7] + cells[pos+8]))
          / local_density;
        /* compute norm of velocity */
        u = sqrt((u_x * u_x) + (u_y * u_y));
        /* compute pressure */
        pressure = local_density * c_sq;
      }
      /* write to file */
      fprintf(fp,"%d %d %.12E %.12E %.12E %.12E %d\n",jj,ii,u_x,u_y,u,pressure,obstacles[ii*params.nx + jj]);
    }
  }

  fclose(fp);

  fp = fopen(AVVELSFILE,"w");
  if (fp == NULL) {
    die("could not open file output file",__LINE__,__FILE__);
  }
  for (ii=0;ii<params.maxIters;ii++) {
    fprintf(fp,"%d:\t%.12E\n", ii, av_vels[ii]);
  }

  fclose(fp);

  return EXIT_SUCCESS;
}

int local_start_calc(int numberOfRows, int size, int rank)
{
  int base,count=0;
  int n = numberOfRows;
  int s = size;

  if (rank >= size) return numberOfRows;
  /*calculate 'base' group size, as numberOfRows/size*/
  base = numberOfRows/size;
  /*subtract base until group size changes*/
  while((n/s)==base && s>1) {
    n = n - base;
    s--;
    count++;
  }
  /*At this point, numberOfRows = (count*base) + ((size-count)*(base+1))
    so first count ranks (0->count-1) should work on base rows, rest on (base+1)*/
  if (rank < (count+1)) {
    return (rank*base);
  }
  else {
    return (count*base) + ((rank-count)*(base+1));
  }
}

void die(const char* message, const int line, const char *file)
{
  fprintf(stderr, "Error at line %d of file %s:\n", line, file);
  fprintf(stderr, "%s\n",message);
  fflush(stderr);
  exit(EXIT_FAILURE);
}

void usage(const char* exe)
{
  fprintf(stderr, "Usage: %s <paramfile> <obstaclefile>\n", exe);
  exit(EXIT_FAILURE);
}
