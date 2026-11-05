---- MODULE lazy_state ----
\* A lazy set kept in a state is tested in the state it was made in; the
\* behavior cannot be fingerprinted, so TLC prints only its exception.
\* options: -simulate -depth 10 -seed 1
EXTENDS Integers
VARIABLES x, s
Init == x = 0 /\ s = {n \in Nat : n > x}
Next == x' = x + 1 /\ s' = {n \in Nat : n > x}
Inv == 2 \in s
====
INIT Init
NEXT Next
INVARIANT Inv
