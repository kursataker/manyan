---- MODULE sf_refine ----
EXTENDS Naturals
VARIABLES x, y
vars == <<x, y>>
Init == x = 0 /\ y = 0
Move == \/ x = 0 /\ x' \in {1, 2} /\ UNCHANGED y
        \/ x # 0 /\ x' = 0 /\ UNCHANGED y
Fire == x = 2 /\ y' = 1 /\ UNCHANGED x
Spec == Init /\ [][Move \/ Fire]_vars /\ WF_vars(Move) /\ SF_vars(Fire)
Fired == <>(y = 1)
====
SPECIFICATION Spec
PROPERTY Fired
