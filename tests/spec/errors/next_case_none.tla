---- MODULE next_case_none ----
\* No CASE guard is true in the next-state relation.
VARIABLE x
Init == x = 0
Next == CASE x = 1 -> x' = 1
====
INIT Init
NEXT Next
