---- MODULE tlcget_level_invariant ----
\* TLCGet fails as TLC starts, so a definition using it is not evaluated
\* once as a constant: the invariant is checked in each state.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x' = x + 1
I == TLCGet("level") < 3
====
INIT Init
NEXT Next
INVARIANT I
