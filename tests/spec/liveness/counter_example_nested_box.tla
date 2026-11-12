---- MODULE counter_example_nested_box ----
\* As TLC, []([]A) is []A and <>(<>A) is <>A: the counterexample takes the
\* cycle once.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = IF x = 2 THEN 1 ELSE x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
P == <>[][](x = 1)
====
SPECIFICATION Spec
PROPERTY P
