---- MODULE enabled ----
EXTENDS Naturals
VARIABLES x, log
A == x < 2 /\ x' = x + 1
Init == x = 0 /\ log = <<>>
Next == \/ A /\ UNCHANGED log
        \/ ~ENABLED A /\ x' = 0 /\ log' = 1
====
INIT Init
NEXT Next
