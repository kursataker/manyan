---- MODULE simulate_stuttering_trace ----
\* options: -simulate num=2 -depth 3 -seed 1
\* TLC checks a simulated behavior without its finite stuttering, here the
\* initial state alone, and builds its product from the tableau's initial
\* nodes there with the edges among them only: no node of []~(x = 2)
\* follows, and the violation is not found.
VARIABLE x
Init == x = 1
Next == x' = 1
Spec == Init /\ [][Next]_x
P == (x = 1) ~> (x = 2)
====
SPECIFICATION Spec
PROPERTY P
