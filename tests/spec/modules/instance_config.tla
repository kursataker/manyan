---- MODULE instance_config ----
\* The configuration may name definitions of an unnamed instance, which
\* see its substitutions: the variable and the constant of this module.
VARIABLE t
INSTANCE Clock WITH N <- 3
====
SPECIFICATION Spec
INVARIANT Bounded
PROPERTY Steps Returns
