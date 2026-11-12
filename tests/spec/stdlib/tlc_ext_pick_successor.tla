---- MODULE tlc_ext_pick_successor ----
\* PickSuccessor asks of each step to a state not yet seen whose
\* condition fails: yes, show the states, the diff, no, explored.
EXTENDS Naturals, TLCExt
VARIABLE x
Init == x = 0
A == x' = x + 1
B == x' = x + 2
Next == A \/ B
Pick == PickSuccessor(x' < 2)
====
INIT Init
NEXT Next
ACTION_CONSTRAINT Pick
