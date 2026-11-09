---- MODULE graph_paths ----
\* CommunityModules' SimplePath, AreConnectedIn and IsStronglyConnected,
\* which TLC implements in Java: an edge is a pair of nodes, and other
\* edges, or those leaving the nodes, are none. Found by tlaplus/Examples'
\* MCEWD687a.
EXTENDS Graphs, TLC, Integers
G == [node |-> 1..4, edge |-> {<<1, 2>>, <<2, 3>>, <<3, 1>>, <<3, 4>>, <<4, 4>>, <<1>>, <<5, 1>>, <<1, 2, 3>>}]
ASSUME PrintT(SimplePath(G))
ASSUME PrintT(<<AreConnectedIn(1, 4, G), AreConnectedIn(4, 1, G), AreConnectedIn(2, 2, G), AreConnectedIn(5, 1, G)>>)
ASSUME PrintT(<<IsStronglyConnected(G), IsStronglyConnected([node |-> 1..3, edge |-> {<<1, 2>>, <<2, 3>>, <<3, 1>>}]),
                IsStronglyConnected([node |-> {}, edge |-> {}])>>)
ASSUME PrintT(SimplePath([node |-> {"a"}, edge |-> {}]))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
