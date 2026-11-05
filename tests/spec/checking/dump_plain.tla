---- MODULE dump_plain ----
\* options: -dump dot graph.dot
\* The state graph in dot, its steps unlabeled, to a deadlock.
EXTENDS Naturals
VARIABLE f
Init == f = [i \in 1..2 |-> "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"]
Next == f[1] = f[2] /\ f' = [f EXCEPT ![1] = "y"]
====
INIT Init
NEXT Next
