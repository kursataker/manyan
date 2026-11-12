---- MODULE counter_example_cycle_repeat ----
\* As TLC, the cycle ends with one step into the state it starts at, not a
\* step to it and a move between its tableau nodes.
VARIABLE s
Succ == <<{2}, {1, 3}, {1, 2, 4}, {2, 4}>>
Init == s \in {1, 2}
Next == s' \in Succ[s]
A0 == s' \in <<{2}, {1, 3}, {2}, {2}>>[s]
A1 == s' \in <<{}, {1}, {1, 2, 4}, {2, 4}>>[s]
Spec == Init /\ [][Next]_s /\ WF_s(A0) /\ SF_s(A1)
P == <>[][s' \in {}]_s
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
