---- MODULE closure ----
\* Graphs grow an edge at a time; the invariant computes reachability.
EXTENDS Naturals, FiniteSets
CONSTANT N
Nodes == 1..N
VARIABLE edges
Init == edges = {}
Next == \/ \E e \in (Nodes \X Nodes) \ edges : edges' = edges \cup {e}
        \/ edges = Nodes \X Nodes /\ UNCHANGED edges
RECURSIVE Reach(_, _)
Reach(S, k) == IF k = 0 THEN S
               ELSE Reach(S \cup {m \in Nodes : \E n \in S : <<n, m>> \in edges}, k - 1)
Closed == \A n \in Nodes :
            LET r == Reach({n}, N) IN
            \A e \in edges : e[1] \in r => e[2] \in r
Sized == Cardinality(edges) <= N * N
====
CONSTANT N = 4
INIT Init
NEXT Next
INVARIANT Closed
INVARIANT Sized
