---- MODULE weak_fails ----
EXTENDS Naturals
VARIABLES x, y
vars == <<x, y>>
Init == x = 0 /\ y = 0
Toggle == x' = 1 - x /\ UNCHANGED y
Finish == x = 1 /\ y = 0 /\ y' = 1 /\ UNCHANGED x
Next == Toggle \/ Finish
Weak == Init /\ [][Next]_vars /\ WF_vars(Toggle) /\ WF_vars(Finish)
Finished == <>(y = 1)
====
SPECIFICATION Weak
PROPERTY Finished
