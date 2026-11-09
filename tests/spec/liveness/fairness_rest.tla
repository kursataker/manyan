---- MODULE fairness_rest ----
\* A conjunct of the specification other than WF and SF, such as
\* <>[][A]_v, holds of every behavior a property is checked on, as in
\* TLC. Found by tlaplus/Examples' MCCRDT.
EXTENDS Naturals
VARIABLE x
Up == x < 3 /\ x' = x + 1
Reset == x' = 0
Spec == x = 0 /\ [][Up \/ Reset]_x /\ WF_x(Up) /\ <>[][Up]_x
Done == <>[](x = 3)
====
SPECIFICATION Spec
PROPERTY Done
