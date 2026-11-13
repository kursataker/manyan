---- MODULE error_initial_tableau_continue ----
\* options: -continue
\* With -continue TLC goes on past the initial state violating the
\* invariant, and fails on its tableau as it stutters, in its search.
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 7}
Inv == x # 7
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x
P == [](x = 0 => <>(IF x = 7 THEN x = "s" ELSE TRUE))
====
SPECIFICATION Spec
PROPERTY P
INVARIANT Inv
CHECK_DEADLOCK FALSE
