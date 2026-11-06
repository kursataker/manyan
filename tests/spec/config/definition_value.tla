---- MODULE definition_value ----
\* The configuration may give a definition a value, which its body then
\* never computes: a number, or a model value for an unbounded CHOOSE.
EXTENDS Naturals, TLC
A == PrintT("A's body") /\ 1
None == CHOOSE n : n \notin Nat
VARIABLE x
Init == x = <<A, None>> /\ PrintT(x)
Next == UNCHANGED x
====
CONSTANTS
  A = 2
  None = None
INIT Init
NEXT Next
