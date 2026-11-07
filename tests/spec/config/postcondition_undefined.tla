---- MODULE postcondition_undefined ----
\* So does a name the spec does not define, or one with parameters.
\* After a violation, as here, TLC ignores that failure.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Post(a) == a
====
INIT Init
NEXT Next
POSTCONDITIONS Post Nope
