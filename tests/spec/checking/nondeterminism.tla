---- MODULE nondeterminism ----
EXTENDS Naturals
VARIABLES x, s
Init == x \in {0, 1} /\ s = {}
Next == \E i \in 1..3 :
          /\ i \notin s
          /\ s' = s \cup {i}
          /\ x' \in {x, 1 - x}
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
