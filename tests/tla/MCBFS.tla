--------------------------------- MODULE MCBFS ---------------------------------
(* Every state graph on three states, with every initial and bad set. *)
EXTENDS BFS
N == 1..3
AllGraphs == [succ : [N -> SUBSET N], init : (SUBSET N) \ {{}}, bad : SUBSET N]
================================================================================
CONSTANT Graphs <- AllGraphs
SPECIFICATION Spec
INVARIANT Complete Shortest Levels
PROPERTY Terminates
CHECK_DEADLOCK FALSE
