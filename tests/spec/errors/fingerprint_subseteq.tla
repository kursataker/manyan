---- MODULE fingerprint_subseteq ----
\* An error inside SUBSET S \subseteq T is one TLC cannot fingerprint:
\* the SUBSET has a source (TLC's EmptySubsetEq).
EXTENDS Integers
VARIABLE b
Init == b = TRUE
Next == b' = /\ (SUBSET (1..3)) \subseteq (1..4)
====
INIT Init
NEXT Next
