---- MODULE instance_assign_definition ----
\* A substitution for a variable may be a definition that is a variable:
\* the instance's m = e assigns x, by default (m == x) or by WITH.
EXTENDS Naturals
VARIABLE x
V == 12
m == x
h == x
M == INSTANCE Counter
N == INSTANCE Counter WITH m <- h
Init == \/ x = 5 \/ M!Start \/ N!Start
Next == M!Step \/ N!Step
====
INIT Init
NEXT Next
