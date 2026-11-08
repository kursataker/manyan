---- MODULE finite_counterexample_fairness ----
\* With fairness the negation has cycle conditions: TLC finds the lasso.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 2 - x
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Stable == [](x = 2 => [](x >= 2))
====
SPECIFICATION Spec
PROPERTY Stable
