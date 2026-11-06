---- MODULE Clock ----
EXTENDS Naturals
CONSTANT N
VARIABLE t
Init == t = 0
Next == t' = (t + 1) % N
Spec == Init /\ [][Next]_t /\ WF_t(Next)
Bounded == t < N
Steps == [][t' # t]_t
Returns == []<>(t = 0)
====
