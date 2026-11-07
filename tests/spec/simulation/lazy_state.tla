---- MODULE lazy_state ----
\* A lazy set kept in a state is tested in the state it was made in. (A
\* violation would hang TLC 1.8.0: comparing the states of its behavior
\* enumerates the set.)
\* options: -simulate num=3 -depth 10 -seed 1
EXTENDS Integers
VARIABLES x, s
Init == x = 0 /\ s = {n \in Nat : n > x}
Next == x' = x + 1 /\ s' = {n \in Nat : n > x}
Inv == x + 1 \in s /\ -1 \notin s
====
INIT Init
NEXT Next
INVARIANT Inv
