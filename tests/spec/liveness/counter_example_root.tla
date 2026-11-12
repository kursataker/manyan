---- MODULE counter_example_root ----
\* The counterexample starts its cycle where TLC's search entered the
\* component, s = 1 reached from 2, not at the initial state 3 in it.
VARIABLE s
Succ == <<{3}, {1}, {1}, {4}, {4, 5}>>
Init == s \in {2, 3, 5}
Next == s' \in Succ[s]
Spec == Init /\ [][Next]_s
P == <>[](s \in {5})
====
SPECIFICATION Spec
PROPERTY P
