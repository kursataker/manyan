---- MODULE finite_counterexample_deadlock_continue ----
\* options: -continue
\* With -continue TLC reports the exception that stops its temporal checks
\* after the deadlock, and the deadlock again as it replays the step.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
NeverThree == [](x = 1 => [](x # 3))
====
INIT Init
NEXT Next
PROPERTY NeverThree
