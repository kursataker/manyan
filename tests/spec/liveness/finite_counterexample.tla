---- MODULE finite_counterexample ----
\* A safety-like property fails on a path back to an expanded state: TLC
\* stops there with a finite behavior, as for an invariant.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 4
Spec == Init /\ [][Next]_x
Stable == [](x = 2 => [](x >= 2))
====
SPECIFICATION Spec
PROPERTY Stable
