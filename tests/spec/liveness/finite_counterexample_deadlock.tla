---- MODULE finite_counterexample_deadlock ----
\* TLC adds a deadlocked state to its temporal checks after reporting the
\* deadlock: a safety-like property violated on the path to it is
\* reported too, with the deadlock's exit status.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
NeverThree == [](x = 1 => [](x # 3))
====
INIT Init
NEXT Next
PROPERTY NeverThree
