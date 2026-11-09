---- MODULE undirected_graphs_record ----
\* ConnectedComponents of no graph is an error, as in TLC's Java.
EXTENDS UndirectedGraphs, TLC, Integers
ASSUME PrintT(ConnectedComponents([node |-> 1..3]))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
