---- MODULE view ----
EXTENDS Naturals
VARIABLES x, steps
Init == x = 0 /\ steps = 0
Next == x' = (x + 1) % 3 /\ steps' = steps + 1
V == x
====
INIT Init
NEXT Next
VIEW V
