---- MODULE liveness_fair ----
\* Temporal properties that hold on every fair behavior.
\* options: -simulate num=5 -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLES x, y
Init == x = 0 /\ y = 0
A == x' = 1 - x /\ UNCHANGED y
B == x = 1 /\ y' = 1 /\ UNCHANGED x
Spec == Init /\ [][A \/ B]_<<x, y>> /\ WF_<<x, y>>(A) /\ SF_<<x, y>>(B)
P == <>(y = 1)
Q == []<><<A \/ B>>_<<x, y>>
====
SPECIFICATION Spec
PROPERTY P Q
