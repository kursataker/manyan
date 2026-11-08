---- MODULE finite_counterexample_search ----
\* TLC stops the search at the finite counterexample, with the counts of
\* that moment, and names each property the behavior violates.
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == \/ x' = (x + 1) % 3 /\ y' = y
        \/ y < 4 /\ y' = y + 1 /\ x' = x
Spec == Init /\ [][Next]_<<x, y>>
Stable == [](x = 2 => [](x >= 2))
Small == [](y < 1 => [](x < 2))
Fine == [](y >= 0)
====
SPECIFICATION Spec
PROPERTY Fine Stable Small
