---- MODULE instance_spec_action ----
\* So is its initial predicate, named by its definition.
\* options: -simulate num=1 -depth 5 -seed 2
EXTENDS Naturals
VARIABLE t
I == INSTANCE C
Spec == I!Spec
Inv == t < 2
====
SPECIFICATION Spec
INVARIANT Inv
