---- MODULE let_action ----
EXTENDS Naturals
VARIABLES x
Init == x = 0
Move(d) == LET nx == x + d IN nx <= 4 /\ x' = nx
Next == Move(1) \/ Move(2) \/ (x = 4 /\ x' = 0)
====
INIT Init
NEXT Next
