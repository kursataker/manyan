---- MODULE counter_example_stuttering_simulate ----
\* options: -simulate num=1 -depth 18 -seed 836
\* In a simulation, a lasso that stutters at its last state steps from it
\* to itself by no action, TLC's UnnamedAction.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x, y
Init == x = 0 /\ y = "a"
Inc == x < 3 /\ x' = x + 1 /\ UNCHANGED y
Flip == y' = "b" /\ UNCHANGED x
Next == Inc \/ Flip
Spec == Init /\ [][Next]_<<x, y>> /\ WF_<<x, y>>(Inc)
Live == <>(y = "b")
Post == PrintT(CounterExample)
====
SPECIFICATION Spec
PROPERTY Live
POSTCONDITION Post
