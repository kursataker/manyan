---- MODULE instance_spec_action ----
\* A specification's step through an instance, which substitutes its
\* variable, is no definition: TLC's Action, at the INSTANCE statement.
EXTENDS Naturals
VARIABLE t
I == INSTANCE C
Spec == I!Spec
Inv == t < 2
====
SPECIFICATION Spec
INVARIANT Inv
