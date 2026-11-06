---- MODULE tlcget_counter ----
\* A simulation keeps none of TLCGet's counters of the search but the time.
\* options: -simulate num=2 -depth 3 -seed 2
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 1}
Next == x < 3 /\ TLCGet("duration") >= 0 /\ PrintT(TLCGet("diameter")) /\ x' \in {x + 1, x + 2}
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
