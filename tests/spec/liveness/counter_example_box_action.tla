---- MODULE counter_example_box_action ----
\* The negation of <>[][A]_s is []<><<~A>>_s: the counterexample's cycle
\* takes a step that is not A, it cannot stutter where one could start.
VARIABLE s
Succ == <<{2, 3}, {1, 3}, {1}>>
Init == s = 2
Next == s' \in Succ[s]
Spec == Init /\ [][Next]_s
P == <>[][s' \in {1, 2}]_s
====
SPECIFICATION Spec
PROPERTY P
