---- MODULE assert ----
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x' = x + 1 /\ Assert(x < 2, "x got too big")
====
INIT Init
NEXT Next
