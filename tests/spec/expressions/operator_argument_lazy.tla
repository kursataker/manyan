---- MODULE operator_argument_lazy ----
\* An operator passed as an argument takes its own arguments lazily, as
\* any definition does: one it does not use is not evaluated.
EXTENDS Naturals
K(a) == 1
Apply(F(_), x) == F(x)
ASSUME Apply(K, CHOOSE n \in {} : TRUE) = 1
====
