---- MODULE finite_counterexample_disjunct ----
\* One disjunct of the negation is free of cycle conditions.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 4
Spec == Init /\ [][Next]_x
Both == [](x = 2 => [](x >= 2)) /\ <>[](x # 5)
====
SPECIFICATION Spec
PROPERTY Both
