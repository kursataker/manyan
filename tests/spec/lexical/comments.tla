Text before the module is ignored, even MODULE words.
---- MODULE comments ----
EXTENDS Naturals, TLC
\* A line comment with (* unbalanced
(* A block comment (* nested *) still a comment *)
X == 1 (* inline *) + 2 \* trailing
-----------------------------------------------------------------------------
ASSUME PrintT(X)
=============================================================================
