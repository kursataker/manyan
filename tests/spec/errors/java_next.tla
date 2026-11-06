---- MODULE java_next ----
\* An error inside an operator TLC implements in Java, in the next-state
\* relation, is an unexpected exception of the model checker.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 + "a"
====
INIT Init
NEXT Next
