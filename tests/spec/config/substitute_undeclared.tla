---- MODULE substitute_undeclared ----
\* A substitution for a name the spec does not have is an error; a value
\* for one is ignored (EC2 from tlaplus-x/tlx).
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 - x
G == 1
====
CONSTANT Unused = 1
CONSTANT Nope <- G
INIT Init
NEXT Next
