---- MODULE next_case_guard_continue ----
\* options: -continue -deadlock
\* With -continue TLC reports an error in Next again as it replays it.
VARIABLE x
Init == x = 0
Next == CASE x = 5 -> x' = 1 [] 1 -> x' = 2
====
INIT Init
NEXT Next
