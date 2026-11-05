---- MODULE if_action ----
EXTENDS Naturals
VARIABLES x, mode
Init == x = 0 /\ mode \in {"up", "down"}
Next == /\ IF mode = "up" THEN x' = (x + 1) % 4 ELSE x' = (x + 3) % 4
        /\ CASE x = 3 -> mode' = "down"
             [] x = 0 -> mode' = "up"
             [] OTHER -> mode' = mode
====
INIT Init
NEXT Next
