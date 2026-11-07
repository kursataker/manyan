---- MODULE action_form ----
\* A temporal formula with an action that is neither <>[]A nor []<>A in
\* the negation is not one TLC can check.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == <><<IF x' = x THEN x = "s" ELSE x' = 9>>_x
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
