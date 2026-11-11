\* options: -length 5
\* apalache-mc check --length=5 --inv=Inv Rec6.tla: ERROR (255)
--------- MODULE Rec6 -----------------
EXTENDS Integers

N == 5

VARIABLES
    \* @type: Set(Int);
    set,
    \* @type: Int;
    count

RECURSIVE Sum(_)

Sum(S) ==
  IF S = {}
  THEN 0
  ELSE LET x == CHOOSE y \in S: TRUE IN
    x + Sum(S \ {x})

UNROLL_DEFAULT_Sum == 0
UNROLL_TIMES_Sum == N

Init ==
  /\ set = {}
  /\ count = 0

Next ==
  \E x \in (1..N) \ set:
    /\ count' = count + x
    /\ set' = set \union {x}

Inv == count = Sum(set)
=======================================    
INIT Init
NEXT Next
INVARIANT Inv
