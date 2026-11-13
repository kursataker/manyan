---- MODULE postcondition_undefined_continue ----
\* options: -continue
\* With -continue TLC reports failing to resolve a postcondition after the
\* deadlock, and the deadlock again as it replays the step.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Post(a) == a
====
INIT Init
NEXT Next
POSTCONDITIONS Post Nope
