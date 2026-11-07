---- MODULE stutter_subscript ----
\* As TLC, <<A>>_v in a temporal property tests v first: A is not
\* evaluated on a stuttering step, where it would fail.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == []<><<IF x' = x THEN x = "s" ELSE x' = 9>>_x
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
