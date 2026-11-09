---- MODULE generate_case ----
\* TLC's -generate cannot choose among a CASE's arms.
\* options: -generate num=1 -depth 3 -seed 1
EXTENDS Integers
VARIABLE x
Init == x = 0
Next == CASE x < 2 -> x' = x + 1
          [] OTHER -> x' = 0
====
INIT Init
NEXT Next
