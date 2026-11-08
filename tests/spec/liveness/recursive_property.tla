---- MODULE recursive_property ----
\* A RECURSIVE property ends through an IF whose guard is constant: TLC
\* translates only the branch it chooses (TLC's Github1389).
EXTENDS Naturals
VARIABLE x
RECURSIVE ap_0(_)
ap_0(u) == IF u THEN <>(x = TRUE) ELSE ap_0(~u)
RECURSIVE ap_3(_)
ap_3(n) == IF n > 0 THEN ap_3(n - 1) ELSE <>(x = TRUE)
Spec == x = TRUE /\ [][x' = ~x]_x
PropIf == ap_0(TRUE)
PropElseRec == ap_3(3)
====
SPECIFICATION Spec
CHECK_DEADLOCK FALSE
PROPERTIES PropIf PropElseRec
