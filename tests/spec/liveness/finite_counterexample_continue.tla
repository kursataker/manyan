---- MODULE finite_counterexample_continue ----
\* options: -continue
\* With -continue TLC reports the exception that stops its temporal checks
\* at a finite counterexample.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 4
Spec == Init /\ [][Next]_x
Both == [](x = 2 => [](x >= 2)) /\ <>[](x # 5)
====
SPECIFICATION Spec
PROPERTY Both
