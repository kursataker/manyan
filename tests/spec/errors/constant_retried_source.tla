---- MODULE constant_retried_source ----
\* TLC fails to normalize this constant definition before the search, so
\* it computes it again where it is used, with sources it then names.
EXTENDS Naturals
VARIABLE x
S == UNION {{1}, Nat} \cup {3}
Init == x \in S
Next == UNCHANGED x
====
INIT Init
NEXT Next
