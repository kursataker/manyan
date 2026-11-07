---- MODULE fingerprint_instance ----
\* An initial state TLC cannot fingerprint, through an INSTANCE that
\* substitutes: the stack starts at the INSTANCE, as TLC enters its
\* substitutions. Found by tlaplus/Examples' TestGraphs.
VARIABLE v
INSTANCE FPM
====
SPECIFICATION Spec
