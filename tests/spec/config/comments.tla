---- MODULE comments ----
EXTENDS Naturals
VARIABLE x
Init == x \in 1..3
Next == UNCHANGED x
Inv == x > 0
====
\* line comment
(* block
   comment *)
INIT
  Init
NEXT Next \* trailing
INVARIANTS Inv
