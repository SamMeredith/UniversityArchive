    import Prelude hiding (lookup)
    import Data.List hiding (lookup)

    type Var = String
    type Pname = String
    type DecV = [(Var,Aexp)]
    {-procedure environment associates each procedure with the effect
     of executing its body-}
    type DecP = [(Pname,Stm)] 
    type Z = Integer
    type T = Bool
    type State = Var -> Z
    type Loc = Z
    type Store = Loc -> Z
    type EnvV = Var -> Loc
    type EnvP = Pname -> Store -> Store
    type Input = [Integer]
    type Output = [String]
    type IOState = (Input, Output, State)

    data Aexp = N Integer | V Var | Add Aexp Aexp | Mult Aexp Aexp
                | Sub Aexp Aexp | UNeg Aexp
      deriving (Show, Read, Eq, Ord)

    data Bexp = TRUE | FALSE | Eq Aexp Aexp | Le Aexp Aexp | Neg Bexp
                | And Bexp Bexp
      deriving (Show, Read, Eq)

    data Stm = Ass Var Aexp | Skip | Comp Stm Stm | If Bexp Stm Stm
               | While Bexp Stm | Block DecV DecP Stm | Call Pname
               | Read Var       -- for reading in the value of a variable
               | WriteA Aexp    -- for writing out the value of an arithmetic expression
               | WriteB Bexp    -- for writing out the value of a Boolean expression
               | WriteS String  -- for writing out a given string
               | WriteLn        -- for writing out a string consisting of a newline character   
      deriving (Show, Read, Eq)

    a_test :: Aexp
    a_test = Add (V "x") (N 1)

    s_init :: State
    s_init "x" = 3
    s_init v = 0

    --Part 1  

    --Evaluate Arithmetic Expressions
    a_val :: Aexp -> State -> Z
    a_val (N n) s = n
    a_val (V x) s = s x
    a_val (Add a1 a2) s = (a_val a1 s) + (a_val a2 s)
    a_val (Mult a1 a2) s = (a_val a1 s) * (a_val a2 s)
    a_val (Sub a1 a2) s = (a_val a1 s) - (a_val a2 s)
    a_val (UNeg a1) s = a_val (Sub (N 0) (a1)) s		--Unary Negation

    --Evaluate Boolean Expressions
    b_val :: Bexp -> State -> T
    b_val TRUE s = True
    b_val FALSE s = False
    b_val (Eq a1 a2) s = (a_val a1 s == a_val a2 s)
    b_val (Le a1 a2) s = (a_val a1 s <= a_val a2 s)
    b_val (Neg b) s = not (b_val b s)
    b_val (And b1 b2) s = (b_val b1 s && b_val b2 s)

   --union could be implemented in prelude as follows
   --union as bs = foldl (\xs y -> if elem y xs then xs else xs ++ [y]) as bs
   --which takes the left most element of 'bs' as y, 'as' as xs

    --Compute set of free variables for arithmetic expressions
    fv_aexp :: Aexp -> [Var]
    fv_aexp (N n) = []
    fv_aexp (V x) = [x]
    fv_aexp (Add a1 a2) = union (fv_aexp a1) (fv_aexp a2)
    fv_aexp (Mult a1 a2) = union (fv_aexp a1) (fv_aexp a2)
    fv_aexp (Sub a1 a2) = union (fv_aexp a1) (fv_aexp a2) 

    --Compute set of free variables for boolean expressions
    fv_bexp :: Bexp -> [Var]
    fv_bexp (Eq a1 a2) = union (fv_aexp a1) (fv_aexp a2)
    fv_bexp (Le a1 a2) = union (fv_aexp a1) (fv_aexp a2)
    fv_bexp (Neg b) = fv_bexp b
    fv_bexp (And b1 b2) = union (fv_bexp b1) (fv_bexp b2)
    fv_bexp b = []

    --Compute set of free variables for statements
    fv_stm :: Stm -> [Var]
    fv_stm (Ass x a1) = union (fv_aexp a1) ([x])
    fv_stm (Skip) = []
    fv_stm (Comp stm1 stm2) = union (fv_stm stm1) (fv_stm stm2)
    fv_stm (If b1 stm1 stm2) = union (union (fv_stm stm1) (fv_stm stm2)) (fv_bexp b1)
    fv_stm (While b1 stm1) = union (fv_stm stm1) (fv_bexp b1)
    fv_stm (Read x) = [x]
    fv_stm (WriteA a1) = fv_aexp a1
    fv_stm (WriteB b1) = fv_bexp b1
    fv_stm (WriteS s) = []
    fv_stm (WriteLn) = []

    fac :: Stm
    fac = Comp
            (WriteS "Factorial calculator")
            (Comp
              (WriteLn)
              (Comp
                (WriteS "Enter number: ")
                (Comp
                  (Read "x")
                  (Comp
                    (WriteS "Factorial of ")
                    (Comp
                      (WriteA (V "x"))
                      (Comp
                        (WriteS " is ")
                        (Comp
                          (Ass "y" (N 1))
                          (Comp
                            (While (Neg (Eq (V "x") (N 1))) (Comp
                              (Ass "y" (Mult (V "y") (V "x")))
                              (Ass "x" (Sub (V "x") (N 1)))
                            ))
                            (Comp
                              (WriteA (V "y"))
                              (Comp
                                (WriteLn)
                                (WriteLn)
                              )
                            )
                          )
                        )
                      )
                    )
                  )
                )
              )
            )

    pow :: Stm
    pow = Comp
            (WriteS "Exponential calculator")
            (Comp
              (WriteLn)
              (Comp
                (WriteS "Enter base: ")
                (Comp
                  (Read "base")
                  (Comp
                    (If (Le (N 1) (V "base")) (Comp
                      (WriteS "Enter exponent: ")
                      (Comp
                        (Read "exponent")
                        (Comp
                          (Ass "num" (N 1))
                          (Comp
                            (Ass "count" (V "exponent"))
                            (Comp
                              (While (Le (N 1) (V "count")) (Comp
                                (Ass "num" (Mult (V "num") (V "base")))
                                (Ass "count" (Sub (V "count") (N 1)))
                              ))
                              (Comp
                                (WriteA (V "base"))
                                (Comp
                                  (WriteS " raised to the power of ")
                                  (Comp
                                    (WriteA (V "exponent"))
                                    (Comp
                                      (WriteS " is ")
                                      (WriteA (V "num"))
                                    )
                                  )
                                )
                              )
                            )
                          )
                        )
                      ))
                      (Comp
                        (WriteS "Invalid base ")
                        (WriteA (V "base"))
                      )
                    )
                    (WriteLn)
                  )
                )
              )
            )

    --define substitution operations on arithmetic expressions
    subst_aexp :: Aexp -> Var -> Aexp -> Aexp
    subst_aexp (V x) v a = if v == x then a else (V x)
    subst_aexp (Add a1 a2) x a = (Add (subst_aexp a1 x a) (subst_aexp a2 x a))
    subst_aexp (Mult a1 a2) x a = (Mult (subst_aexp a1 x a) (subst_aexp a2 x a))
    subst_aexp (Sub a1 a2) x a = (Sub (subst_aexp a1 x a) (subst_aexp a2 x a))
    subst_aexp a _ _ = a

    --define substitution operations on boolean expressions
    subst_bexp :: Bexp -> Var -> Aexp -> Bexp
    subst_bexp (Eq a1 a2) x a = (Eq (subst_aexp a1 x a) (subst_aexp a2 x a))
    subst_bexp (Le a1 a2) x a = (Le (subst_aexp a1 x a) (subst_aexp a2 x a))
    subst_bexp (Neg b) x a = (Neg (subst_bexp b x a))
    subst_bexp (And b1 b2) x a = (And (subst_bexp b1 x a) (subst_bexp b2 x a))
    subst_bexp b _ _ = b

    --define the conditional function
    cond :: (a -> T, a -> a, a -> a) -> (a -> a)
    cond (b,f,g) s = if (b s) then (f s) else (g s)

    --update a state
    update :: Eq a => (a -> b) -> b -> a -> (a -> b)
    update s v x y = if (x == y) then v else s y

    --fixpoint operator
    fix :: (a -> a) -> a  {-((a -> a) -> (a -> a)) -> (a -> a) as defined in coursework, but less general?-} 
    fix f = f (fix f)

    --Evaluate statements
    s_ds :: Stm -> State -> State
    s_ds (Ass x a) s = update s (a_val a s) x
    s_ds Skip s = s
    s_ds (Comp ss1 ss2) s = ((s_ds ss2) . (s_ds ss1)) s
    s_ds (If b ss1 ss2) s = cond ((b_val b),(s_ds ss1),(s_ds ss2)) s
    s_ds (While b ss) s = (fix ff) s
      where ff g = cond ((b_val b),(g . s_ds ss),(id))
    s_ds _ _ = error "Cannot evaluate statements of that form"  

    s_ds2 :: Stm -> IOState -> IOState
    s_ds2 (Ass x a) (i, o, s) = (i, o, (update s (a_val a s) x))
    s_ds2 (Skip) ios = ios
    s_ds2 (Comp ss1 ss2) (i, o, s) = (((s_ds2 ss2) . (s_ds2 ss1)) (i, o, s))
    s_ds2 (If b ss1 ss2) (i, o, s) = cond ((\(i,o,s) -> b_val b s),(s_ds2 ss1),(s_ds2 ss2)) (i, o, s)
    s_ds2 (While b ss) (i, o, s) = (fix ff) (i, o, s)
      where ff g = cond ((\(i,o,s) -> b_val b s),(g . s_ds2 ss), (id))
    s_ds2 (Read x) (i:is, o, s) = (is, o ++ ["<" ++ (show i) ++ ">"], (s_ds (Ass x (N i)) s))
    s_ds2 (WriteA a) (i, o, s) = (i, o ++ [(show (a_val a s))], s)
    s_ds2 (WriteB b) (i, o, s) = (i, o ++ [(show (b_val b s))], s)
    s_ds2 (WriteS str) (i, o, s) = (i, o ++ [str], s)
    s_ds2 (WriteLn) (i, o, s) = (i, o ++ ["\n"], s)

    eval_help :: [Var] -> IOState -> (Input, Output, [Var], [Z])
    eval_help vs (i, o, s) = (i, o, vs, (map (\v -> s v) vs))

    eval :: Stm -> IOState -> (Input, Output, [Var], [Z])
    eval ss (i, o, s) = eval_help (fv_stm ss) (s_ds2 ss (i, o, s))

    --Calculate y = x!
    p :: Stm
    p = Comp
          (Comp 
            (Ass 
              ("x")
              (N 5)) 
            (Ass
              ("y")
              (N 1))) 
          (While
            (Neg
              (Eq 
                (V "x") 
                (N 1)))
            (Comp 
              (Ass 
                ("y") 
                (Mult 
                  (V "y") 
                  (V "x")))
              (Ass 
                ("x") 
                (Sub 
                  (V "x") 
                  (N 1)))))

    p' :: (Z,Z)
    p' = (1, 120)

    s_test :: State
    s_test "x" = 40
    s_test "y" = 5
    s_test v = 3

    s_finalp = s_ds p s_test
    
    {-Exercises

    myDiv = Comp ( Ass ("z") (N 0) ) ( While (Le (V "y") (V "x"))
            (Comp (Ass ("z") (Add (V "z") (N 1))) (Ass ("x")
            (Sub (V "x") (V "y")))))

    s_finaldiv = s_ds myDiv s_test

    --Exercise B.2
    nPowerm = Comp (Ass "x" (N 1)) (While (Neg (Le (V "m") (N 0))) (Comp (Ass "x" 
              ((Mult (V "x") (V "n")))) (Ass "m" ((Sub (V "m") (N 1))))))  

    s_final = s_ds nPowerm s_test
	
    -}

    -- *********************************************************
    --Part 2

    --Evaluate the sucessor of a location
    new :: Loc -> Loc
    new n = n + 1

    --Find the next location
    next :: Loc
    next = 0

    --Lookup the state in a given store and environment
    lookup :: EnvV -> Store -> State
    lookup envv sto = sto . envv

    --Variable environment update
    d_v_ds :: DecV -> (EnvV, Store) -> (EnvV, Store)
    d_v_ds [] (envv, sto) = (envv, sto)
    d_v_ds ((v, a):vs) (envv, sto) = d_v_ds vs ((update envv (sto next) v),
      (update (update sto (a_val a (lookup envv sto)) (sto next))
      (new (sto next)) (next)))

    --Procedure environment update
    d_p_ds :: DecP -> EnvV -> EnvP -> EnvP
    d_p_ds [] envv envp = envp
    d_p_ds ((p, ss):ps) envv envp = d_p_ds ps envv (update envp (fix ff) p)
      where ff g = s_static ss envv (update envp g p)

    --Evaluate statement using static variables
    s_static :: Stm -> EnvV -> EnvP -> Store -> Store
    s_static (Ass x a) envv envp sto =
      update sto (a_val a (lookup envv sto)) (envv x)
    s_static Skip envv envp sto = sto
    s_static (Comp ss1 ss2) envv envp sto =
      ((s_static ss2 envv envp) . (s_static ss1 envv envp)) sto
    s_static (If b ss1 ss2) envv envp sto =
      cond ((b_val b).(lookup envv), (s_static ss1 envv envp),
      (s_static ss2 envv envp)) sto
    s_static (While b ss) envv envp sto = (fix ff) sto
      where ff g = cond ((b_val b).(lookup envv), (g . (s_static ss envv envp)), id)
    s_static (Block decv decp ss) envv envp sto = s_static ss envv' envp' sto'
      where (envv', sto') =  d_v_ds decv (envv, sto)
            envp' = d_p_ds decp envv' envp
    s_static (Call p) envv envp sto = envp p sto

    --factorial with static variables
    q :: Stm
    q = Block [("x", (N 5)), ("y", (N 1))]
          [("fac", Block [("z", (V "x"))] [] 
             (If 
               (Eq (V "x") (N 1))
               (Skip)
               (Comp
                 (Comp 
                   (Ass "x" 
                     (Sub
                       (V "x")
                       (N 1)))
                   (Call "fac"))
                 (Ass "y" (Mult (V "z") (V "y"))))))]
          (Call "fac")

    {-
	f :: DecP
    f = [("fac", Block [("z", (V "x"))] [] 
           (If 
             (Eq (V "x") (N 1))
             (Skip)
             (Comp
               (Comp 
                 (Ass "x" 
                   (Sub
                     (V "x")
                     (N 1)))
                 (Call "fac"))
               (Ass "y" (Mult (V "z") (V "y"))))))]

    f1 :: DecV
    f1 = [("x", (N 4)), ("y", (N 1))]

    f2 :: Stm
    f2 = Block f1 f (Call "fac")

    f_test :: Integer
    f_test = s_static f2 undefined undefined t 0
	
    var_test :: Stm
    var_test = Block [("x", (N 1))] [] (Ass "x" (N 0))

    v_test :: Integer
    v_test = s_static var_test undefined undefined t 0
	-}

    --minimal store with next (0) mapped to 1
    t :: Store
    t 0 = 1
    t loc = undefined

    q' :: [Z]
    q' = map (s_static q undefined undefined t) [0..]

    -- ********************************************************
    --Part 3

    r :: Stm
    r = Block [("x", (N 0))]
          [("p", (Ass "x" (Mult (V "x") (N 2)))), ("q", (Call "p"))]
            (Block [("x", (N 5))]
              [("p", (Ass "x" (Add (V "x") (N 1))))]
                (Call "q"))

    tstm :: Stm 
    tstm = Block [("x", (N 0))]
             [("p", (Ass "x" (Add (V "x") (N 1)))),
             ("q", (Call "p"))]
               (Block [] [("p", (Ass "x" (N 7)))]
                 (Call "q"))

    findr1 = map (s_static r undefined undefined t) [0..]

    r1 :: (Z,Z)
    r1 = (0,5)

    findr2 = map (s_mixed r undefined undefined t) [0..]

    r2 :: (Z,Z)
    r2 = (0,10)

    findr3 = map (s_dynamic r undefined undefined t) [0..]

    r3 :: (Z,Z)
    r3 = (0,6)

    -- **********************************************************
    --Mixed bindings

    type EnvP' = Pname -> EnvV -> Store -> Store

    d_p'_ds :: DecP -> EnvV -> EnvP' -> EnvP'
    d_p'_ds [] envv envp = envp
    d_p'_ds ((p, ss):ps) envv envp = d_p'_ds ps envv (update envp (fix ff) p)
      where ff g = (\envv' -> s_mixed ss envv' (update envp g p))
      {-(update envp (\envv' -> s_mixed ss envv' envp) p)-} --non-recursive definition

    --Procedure environment waits to be given the current Variable environment
    s_mixed :: Stm -> EnvV -> EnvP' -> Store -> Store
    s_mixed (Ass x a) envv envp sto =
      update sto (a_val a (lookup envv sto)) (envv x)
    s_mixed Skip envv envp sto = sto
    s_mixed (Comp ss1 ss2) envv envp sto =
      ((s_mixed ss2 envv envp) . (s_mixed ss1 envv envp)) sto
    s_mixed (If b ss1 ss2) envv envp sto =
      cond ((b_val b).(lookup envv), (s_mixed ss1 envv envp),
      (s_mixed ss2 envv envp)) sto
    s_mixed (While b ss) envv envp sto = (fix ff) sto
      where ff g = cond ((b_val b).(lookup envv), (g . (s_mixed ss envv envp)), id)
    s_mixed (Block decv decp ss) envv envp sto = s_mixed ss envv' envp' sto'
      where (envv', sto') =  d_v_ds decv (envv, sto)
            envp' = d_p'_ds decp envv' envp
    s_mixed (Call p) envv envp sto = envp p envv sto

    -- ***************************************************************
    --Dynamic binding

    --EnvP'' is a function that takes a (thing below that we actually want)
    --and returns an EnvP'' that we can use
    newtype EnvP'' = EnvP'' (Pname -> EnvV -> EnvP'' -> Store -> Store)

    --unwrap EnvP'' one level to get the useful thing inside, update that as
    --normal, then wrap up again
    d_p''_ds :: DecP -> EnvV -> EnvP'' -> EnvP''
    d_p''_ds [] envv envp = envp
    d_p''_ds ((p, ss):ps) envv (EnvP'' envp) = d_p''_ds ps envv
      (EnvP'' (update envp (\envv' envp' -> s_dynamic ss envv' envp') p))

    --Only call changed, unwrap EnvP'' one stage the get useful function
    --use this function (passing as arguments the current variable environment,
    --the current procedure environment and the procedure to be evaluated)
    --to get store
    s_dynamic :: Stm -> EnvV -> EnvP'' -> Store -> Store
    s_dynamic (Ass x a) envv envp sto =
      update sto (a_val a (lookup envv sto)) (envv x)
    s_dynamic Skip envv envp sto = sto
    s_dynamic (Comp ss1 ss2) envv envp sto =
      ((s_dynamic ss2 envv envp) . (s_dynamic ss1 envv envp)) sto
    s_dynamic (If b ss1 ss2) envv envp sto =
      cond ((b_val b).(lookup envv), (s_dynamic ss1 envv envp),
      (s_dynamic ss2 envv envp)) sto
    s_dynamic (While b ss) envv envp sto = (fix ff) sto
      where ff g = cond ((b_val b).(lookup envv), (g . (s_dynamic ss envv envp)), id)
    s_dynamic (Block decv decp ss) envv envp sto = s_dynamic ss envv' envp' sto'
      where (envv', sto') =  d_v_ds decv (envv, sto)
            envp' = d_p''_ds decp envv envp
    s_dynamic (Call p) envv (EnvP'' envp) sto = envp p envv (EnvP'' envp) sto


