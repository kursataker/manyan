---- MODULE tlcget_level ----
\* TLCGet("level") is the length of the behavior so far, in the actions
\* and in the invariants of a successor.
\* options: -simulate num=1 -depth 10 -seed 1
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == PrintT(<<x, TLCGet("level")>>) /\ x' = x + 1
I == TLCGet("level") < 3
====
INIT Init
NEXT Next
INVARIANT I
