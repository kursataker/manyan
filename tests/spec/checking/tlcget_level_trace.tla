---- MODULE tlcget_level_trace ----
\* As TLC rebuilds a trace, TLCGet("level") is each step's, not the
\* level the search stopped at.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == PrintT(<<x, TLCGet("level")>>) /\ x' = x + 1
I == TLCGet("level") < 3
====
INIT Init
NEXT Next
INVARIANT I
