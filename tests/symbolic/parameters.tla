---- MODULE parameters ----
\* An operator's parameter stands for the variable its argument names, which
\* the body primes or keeps UNCHANGED.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y
Up(v) == v < 4 /\ v' = v + 1
Keep(v) == UNCHANGED v
Init == x = 0 /\ y = 0
Next == \/ Up(x) /\ Keep(y)
        \/ Up(y) /\ Keep(x)
Inv == x + y < 5
====
INIT Init
NEXT Next
INVARIANT Inv
