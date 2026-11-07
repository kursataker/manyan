---- MODULE instance_init ----
\* An initial predicate through an instance that substitutes is named as
\* its action would be, at the INSTANCE statement.
\* options: -simulate num=5 -depth 19 -seed 185
EXTENDS Naturals
VARIABLE t
INSTANCE Clock WITH N <- 3
Small == t < 2
====
INIT Init
NEXT Next
INVARIANT Small
