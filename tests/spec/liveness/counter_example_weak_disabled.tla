---- MODULE counter_example_weak_disabled ----
\* Weak fairness on A holds in the cycle a, b only because A is disabled
\* in b: the counterexample's cycle goes through b, not stuttering in a.
VARIABLE x
Init == x = "a"
A == x = "a" /\ x' = "c"
B == (x = "a" /\ x' = "b") \/ (x = "b" /\ x' = "a")
Next == A \/ B \/ (x = "c" /\ UNCHANGED x)
Spec == Init /\ [][Next]_x /\ WF_x(A)
P == <>(x = "c")
====
SPECIFICATION Spec
PROPERTY P
