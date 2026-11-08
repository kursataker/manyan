---- MODULE Queue ----
EXTENDS Naturals, Sequences
CONSTANT Max
VARIABLE q
TypeOK == q \in Seq(1..2) /\ Len(q) <= Max
Size == Len(q)
Next == \/ Len(q) < Max /\ \E m \in 1..2 : q' = Append(q, m)
        \/ q # <<>> /\ q' = Tail(q)
Spec == q = <<>> /\ [][Next]_q
====
