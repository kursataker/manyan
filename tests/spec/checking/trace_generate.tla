---- MODULE trace_generate ----
\* options: -generate num=1 -depth 5 -seed 1
\* A generated behavior's Trace: each state a record with its action, which
\* Json's Java writes first, before the variables.
EXTENDS TLC, TLCExt, Json, Naturals
VARIABLES zz, b
Init == zz = 0 /\ b = 1
Next == zz < 1 /\ zz' = zz + 1 /\ b' = b
Inv == zz < 1 \/ (PrintT(ToJson(Trace)) /\ FALSE)
====
INIT Init
NEXT Next
INVARIANT Inv
