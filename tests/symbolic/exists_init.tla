---- MODULE exists_init ----
\* \E in the initial predicate, with a guard over two variables.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y
Init == \E k \in 1..5 : x = k /\ y = k * k
Next == UNCHANGED <<x, y>>
Inv == y /= 16
====
INIT Init
NEXT Next
INVARIANT Inv
