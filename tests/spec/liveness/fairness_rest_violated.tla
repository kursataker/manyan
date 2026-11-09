---- MODULE fairness_rest_violated ----
\* <>[][A]_v alone leaves stuttering possible: the property fails, with
\* the counterexample TLC gives.
EXTENDS Naturals
VARIABLE x
Up == x < 3 /\ x' = x + 1
Reset == x' = 0
Spec == x = 0 /\ [][Up \/ Reset]_x /\ <>[][Up]_x
Done == <>[](x = 3)
====
SPECIFICATION Spec
PROPERTY Done
