---- MODULE assume_level ----
\* An assumption must be a constant: SANY's level error, located from its
\* keyword.
EXTENDS Naturals
VARIABLE x
ASSUME Named == x' = 0
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
