---- MODULE graph_paths_record ----
\* AreConnectedIn in no graph is an error, as in TLC's Java.
EXTENDS Graphs, TLC, Integers
ASSUME PrintT(AreConnectedIn(1, 2, [node |-> 1..3, edge |-> 7]))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
