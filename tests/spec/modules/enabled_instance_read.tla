---- MODULE enabled_instance_read ----
\* Inside ENABLED, a primed variable of the instance that the action
\* assigned by name is read back by name, not through its substitution.
EXTENDS Naturals, Sequences, TLC
VARIABLES b, s
Svc == INSTANCE Inner WITH c <- s, a <- b
Init == b = 0 /\ s = <<1, 2, 3>>
Next == UNCHANGED <<b, s>>
Inv == PrintT(<<Svc!Same, Svc!Filtered>>)
====
INIT Init
NEXT Next
INVARIANT Inv
