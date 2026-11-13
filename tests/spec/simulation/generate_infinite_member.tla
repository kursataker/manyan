---- MODULE generate_infinite_member ----
\* options: -generate num=5 -depth 22 -seed 379
\* -generate draws a successor x' \in S at random: TLC counts S first, and
\* fails counting [{1} -> Nat] at Nat.
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
