---- MODULE fixed_set_source ----
\* Sets made of constants alone, computed once, still name where they are
\* written when enumerating one fails, as a set made anew does in TLC.
EXTENDS Naturals
CONSTANT N
VARIABLE x
Init == x = {}
Next == \/ \E s \in SUBSET (1..N) : x' = s
        \/ x = 1..N /\ x' \in [{1} -> Nat]
====
CONSTANT N = 3
INIT Init
NEXT Next
