---- MODULE view_constant ----
\* A view that is a constant is TLC's error.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
View == <<1>>
====
INIT Init
NEXT Next
VIEW View
