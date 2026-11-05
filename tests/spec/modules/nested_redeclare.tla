---- MODULE nested_redeclare ----
\* A nested module declares again constants and a variable of the module
\* around it: SANY warns of each, in order, and the model is checked.
EXTENDS Naturals
CONSTANTS C, D
VARIABLE x
---- MODULE Inner ----
CONSTANTS D, C
VARIABLE x
Sum == C + D
====
I == INSTANCE Inner
Init == x = I!Sum
Next == x' = x
====
CONSTANTS C = 1 D = 2
INIT Init
NEXT Next
