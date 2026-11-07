---- MODULE stutter_promise ----
\* So too of a specification without fairness, when a promise of the
\* property's tableau has a []: TLC warns the fairness may be missing.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
Prop == [](x = 1 => <>(x = 9))
====
SPECIFICATION Spec
PROPERTY Prop
CHECK_DEADLOCK FALSE
