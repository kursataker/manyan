------------------------------ MODULE MCInitSlices -----------------------------
(* Every Init of choices three deep, each of one or two alternatives, its
   paths failing or ending in one of two states, sliced to each depth. *)
EXTENDS InitSlices
Leaves == {[s |-> v] : v \in 0..2}
Grow(T) == Leaves \cup {[alts |-> a] : a \in [1..1 -> T] \cup [1..2 -> T]}
AllTrees == Grow(Grow(Grow(Leaves)))
================================================================================
CONSTANT Trees <- AllTrees
CONSTANT Depths = {1, 2, 3}
SPECIFICATION Spec
INVARIANT Same
