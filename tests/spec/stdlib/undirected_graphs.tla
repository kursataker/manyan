---- MODULE undirected_graphs ----
\* CommunityModules' ConnectedComponents, which TLC implements in Java: an
\* edge is a set of one or two nodes, and other edges, or those leaving the
\* nodes, join nothing. Found by tlaplus/Examples' YoYoAllGraphs.
EXTENDS UndirectedGraphs, TLC, Integers
ASSUME PrintT(ConnectedComponents([node |-> 1..4, edge |-> {{1, 3}, {2, 4}}]))
ASSUME PrintT(ConnectedComponents([node |-> 1..3, edge |-> {{1, 3}, {2, 3}}]))
ASSUME PrintT(ConnectedComponents([node |-> 1..5, edge |-> {{1}, {1, 2, 3}, {3, 9}, {}, {4, 5}}]))
ASSUME PrintT(ConnectedComponents([node |-> {}, edge |-> {}]))
ASSUME PrintT({S \in SUBSET {{1, 2}, {1, 3}, {2, 3}} : IsStronglyConnected([node |-> 1..3, edge |-> S])})
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
