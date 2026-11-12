---- MODULE tlc_ext_pick_successor_vars ----
\* PickSuccessor's states and diff of two variables, and an empty answer,
\* which is yes.
EXTENDS Naturals, TLCExt
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == \/ x' = x + 1 /\ UNCHANGED y
        \/ \E k \in {1, 2} : y' = y + k /\ UNCHANGED x
Pick == PickSuccessor(x' + y' < 2)
Small == x + y < 4
====
INIT Init
NEXT Next
ACTION_CONSTRAINT Pick
CONSTRAINT Small
