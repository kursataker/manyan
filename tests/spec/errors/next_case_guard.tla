---- MODULE next_case_guard ----
\* A CASE guard in the next-state relation is not a boolean: TLC names the arm.
VARIABLE x
Init == x = 0
Next == CASE x = 5 -> x' = 1 [] 1 -> x' = 2
====
INIT Init
NEXT Next
