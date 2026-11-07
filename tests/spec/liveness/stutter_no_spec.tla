---- MODULE stutter_no_spec ----
\* A counterexample that stutters, of INIT and NEXT, which TLC lets stutter
\* forever: TLC warns there is no SPECIFICATION.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Prop == []<>(x = 9)
====
INIT Init
NEXT Next
PROPERTY Prop
CHECK_DEADLOCK FALSE
