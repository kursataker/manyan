---- MODULE fairness_implies ----
\* A fairness conjunct P => WF_v(A) with P constant is WF_v(A) where P
\* holds and nothing where it does not, as TLC evaluates P. Found by
\* tlaplus/Examples' MCInnerSerial.
EXTENDS Naturals
VARIABLE x
Next == x' = (x + 1) % 3
Spec == x = 0 /\ [][Next]_x /\ \A i \in {1, 2} : (i # 1) => WF_x(Next)
Zero == []<>(x = 0)
====
SPECIFICATION Spec
PROPERTY Zero
