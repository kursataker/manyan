---- MODULE tlc_ext_defer ----
\* Model checking evaluates no TLCDefer: it is TRUE.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1 /\ TLCDefer(PrintT(<<"defer", x>>))
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
