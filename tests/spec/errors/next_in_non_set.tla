---- MODULE next_in_non_set ----
\* A variable assigned from a non-set in the next-state relation.
VARIABLE x
Init == x = 0
Next == x' \in 1
====
INIT Init
NEXT Next
