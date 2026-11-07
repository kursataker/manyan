---- MODULE tlcget_counter ----
\* In a simulation TLCGet("diameter") counts the behaviors, from 1.
\* options: -simulate num=2 -depth 3 -seed 2
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 1}
Next == x < 3 /\ TLCGet("duration") >= 0 /\ PrintT(TLCGet("diameter")) /\ x' \in {x + 1, x + 2}
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
