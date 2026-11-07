---- MODULE init_error ----
\* As the simulator computes the initial states, a TLCRuntimeException is
\* an unexpected exception, as any error there.
\* options: -simulate -depth 3 -seed 2
EXTENDS Naturals
VARIABLE x
Init == x = 1 + "a"
Next == x' = x
====
INIT Init
NEXT Next
