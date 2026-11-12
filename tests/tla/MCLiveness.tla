------------------------------ MODULE MCLiveness -------------------------------
(* Every product graph on three nodes from node 1, the eventuality fulfilled
   at every node or at node 3, under no fairness condition, one with its
   steps the edges into some nodes, or two without steps. *)
EXTENDS Liveness
N == 1..3
Into(T) == {e \in N \X N : e[2] \in T}
One == [strong : BOOLEAN, enabled : SUBSET N, step : {Into(T) : T \in SUBSET N}]
Bare == [strong : BOOLEAN, enabled : SUBSET N, step : {{}}]
AllGraphs == [succ : [N -> SUBSET N], init : {{1}}, done : {N, {3}},
              fairs : {<<>>} \cup {<<f>> : f \in One} \cup {<<f, h>> : f, h \in Bare}]
================================================================================
CONSTANT Graphs <- AllGraphs
SPECIFICATION Spec
INVARIANT Complete LassoFair
