---- MODULE tlc_ext_pick_successor_error ----
\* PickSuccessor of no boolean.
EXTENDS Naturals, TLCExt
VARIABLE x
Init == x = 0
Next == x' = x + 1
Pick == PickSuccessor(x' + 1)
====
INIT Init
NEXT Next
ACTION_CONSTRAINT Pick
