---- MODULE nested_warnings ----
\* SANY prints its warnings after each module it processes, all so far:
\* M's after M, then M's and this module's after this one.
EXTENDS Naturals
CONSTANT D
---- MODULE In ----
CONSTANT D
====
I == INSTANCE M WITH C <- 1
VARIABLE x
Init == x = D
Next == x' = x
====
CONSTANT D = 1
INIT Init
NEXT Next
