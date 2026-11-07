---- MODULE tlcset_exit ----
\* TLCSet("exit", TRUE) ends the simulation after the step that sets it,
\* with no error.
\* options: -simulate num=3 -depth 20 -seed 1
EXTENDS Naturals, TLC
VARIABLE n
Init == n = 0
Next == n' = n + 1 /\ (n = 3 => TLCSet("exit", TRUE))
Inv == n < 8
====
INIT Init
NEXT Next
INVARIANT Inv
