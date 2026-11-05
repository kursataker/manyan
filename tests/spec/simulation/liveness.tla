---- MODULE liveness ----
\* A temporal property that fails on a behavior: it may stutter in any state.
\* options: -simulate num=5 -depth 10 -seed 1
EXTENDS Integers
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x
P == <>(x = 5)
====
SPECIFICATION Spec
PROPERTY P
