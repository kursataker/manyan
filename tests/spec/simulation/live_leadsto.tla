---- MODULE live_leadsto ----
\* options: -simulate num=1 -depth 25 -seed 619
EXTENDS Naturals
VARIABLES x, y
vars == <<x, y>>
Init == x = 0 /\ y = 0
Move == \/ x = 0 /\ x' \in {1, 2, 3} /\ UNCHANGED y
        \/ x # 0 /\ x' = (x * 2) % 4 /\ UNCHANGED y
        \/ x = 3 /\ x' = 0 /\ UNCHANGED y
Fire == x = 2 /\ y' = 1 - y /\ UNCHANGED x
Spec == Init /\ [][Move \/ Fire]_vars /\ WF_vars(Move) /\ SF_vars(Fire)
P == (x = 2) ~> (y = 1)
====
SPECIFICATION Spec
PROPERTY P
