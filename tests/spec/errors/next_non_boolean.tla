---- MODULE next_non_boolean ----
\* A conjunct of the next-state relation that is not a boolean: TLC shows the value.
VARIABLE x
Init == x = 0
Next == \E y \in {1} : y
====
INIT Init
NEXT Next
