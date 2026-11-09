---- MODULE Graphs ----
\* CommunityModules' path operators, with their definitions: TLC replaces
\* them by its Java, as tpp by its own.
LOCAL INSTANCE Naturals
LOCAL INSTANCE Sequences
Path(G) == {p \in Seq(G.node) :
             /\ p # << >>
             /\ \A i \in 1..(Len(p)-1) : <<p[i], p[i+1]>> \in G.edge}
SimplePath(G) == { p \in Path(G) : \A i,j \in 1..Len(p) : p[i] = p[j] => i = j }
AreConnectedIn(m, n, G) == \E p \in Path(G) : (p[1] = m) /\ (p[Len(p)] = n)
IsStronglyConnected(G) == \A m, n \in G.node : AreConnectedIn(m, n, G)
====
