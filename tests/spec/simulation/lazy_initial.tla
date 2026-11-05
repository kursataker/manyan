---- MODULE lazy_initial ----
\* A lazy function made by the initial predicate reads the state it was made in.
\* options: -simulate num=3 -depth 5 -seed 1
EXTENDS Integers
VARIABLES x, f
Init == x = 0 /\ f = [n \in Nat |-> n + x]
Next == x' = x + 1 /\ f' = f
Inv == f[0] = 0
====
INIT Init
NEXT Next
INVARIANT Inv
