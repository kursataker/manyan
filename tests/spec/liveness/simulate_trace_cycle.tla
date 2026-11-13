---- MODULE simulate_trace_cycle ----
\* options: -simulate num=1 -depth 5 -seed 1
\* A simulated behavior that comes back to its states: TLC's product of it
\* steps on from the nodes new at each position, and the last x = 2
\* stutters in a node with nothing left to do.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x
P == [](x = 2 => [](x >= 2))
====
SPECIFICATION Spec
PROPERTY P
