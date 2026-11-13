---- MODULE unassigned_simulate_continue ----
\* options: -simulate num=3 -depth 3 -seed 1 -continue
\* With -continue the simulator sets up no postcondition writing the
\* behavior, which would fail on the successor left incomplete.
VARIABLES x, y
Init == x = 1 /\ y = 1
Next == x' = 2
====
INIT Init
NEXT Next
