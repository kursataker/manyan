---- MODULE tlcget_assume ----
\* The counters of a search are undefined in a simulation's assumptions.
\* options: -simulate num=1 -depth 3 -seed 1
EXTENDS Naturals, TLC
VARIABLE x
ASSUME PrintT(TLCGet("distinct"))
Init == x = 0
Next == x' = x
====
INIT Init
NEXT Next
