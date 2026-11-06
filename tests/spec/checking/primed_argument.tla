---- MODULE primed_argument ----
\* A parameter may stand for a primed variable, which an equality in the
\* body then assigns; so may that of a constant operator the configuration
\* replaces by a definition.
EXTENDS Naturals
CONSTANT Send(_, _)
VARIABLES x, y
Set(new, v) == new = v
MCSend(v, new) == new \in {v, v + 1}
Init == x = 0 /\ y = 0
Next == x < 3 /\ Set(x', x + 1) /\ Send(x, y')
====
CONSTANT Send <- MCSend
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
