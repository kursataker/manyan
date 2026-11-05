---- MODULE liveness_last ----
\* A behavior that ends without a successor stutters there.
\* options: -simulate num=3 -depth 10 -seed 1
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
P == <>(x = 5)
====
SPECIFICATION Spec
PROPERTY P
CHECK_DEADLOCK FALSE
