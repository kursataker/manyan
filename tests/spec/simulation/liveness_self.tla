---- MODULE liveness_self ----
\* A step that changes nothing is stuttering in a counterexample.
\* options: -simulate num=30 -depth 5 -seed 9
EXTENDS Integers, Sequences, TLC
VARIABLE x
Init == x \in 0..3
Next == x' = x
Spec == Init /\ [][Next]_x /\ WF_x(Next)
P == <>(x < 3)
====
SPECIFICATION Spec
PROPERTY P
