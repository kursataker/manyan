---- MODULE UndirectedGraphs ----
\* CommunityModules' ConnectedComponents, with its definition: TLC
\* replaces it by its Java, as Manyan by its own.
LOCAL INSTANCE Naturals
LOCAL INSTANCE Sequences
ConnectedComponents(G) ==
    LET IsCC(S) == /\ S # {}
                   /\ \A m,n \in S : \E p \in Seq(S) :
                         /\ p # << >>
                         /\ p[1] = m /\ p[Len(p)] = n
                         /\ \A i \in 1 .. Len(p)-1 : {p[i], p[i+1]} \in G.edge
    IN  { S \in SUBSET G.node :
            /\ IsCC(S)
            /\ \A T \in SUBSET G.node : S \subseteq T /\ S # T => ~ IsCC(T)
        }
IsStronglyConnected(G) == ConnectedComponents(G) = {G.node}
====
