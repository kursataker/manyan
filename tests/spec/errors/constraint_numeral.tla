---- MODULE constraint_numeral ----
\* A constraint that is not a boolean, named by its numeral as TLC names it.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 - x
C == 3
====
INIT Init
NEXT Next
CONSTRAINT C
