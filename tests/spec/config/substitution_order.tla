---- MODULE substitution_order ----
\* Of several constants substituted by undefined names, TLC names the
\* first in the order of SANY's hash table of the module's symbols
\* (SASwap from dgpv/SASwap_TLAplus_spec).
EXTENDS Naturals, Sequences, FiniteSets, TLC
CONSTANTS Delta, Epsilon, Beta, Gamma, Alpha
VARIABLE x
Init == x = 0
Next == x' = x
====
CONSTANTS Delta <- D Epsilon <- E Beta <- B Gamma <- G Alpha <- A
INIT Init
NEXT Next
