---- MODULE counter_example ----
\* A simulation's CounterExample is the behavior to its error, its states
\* the ALIAS's; a lasso's steps back.
\* options: -simulate num=3 -depth 6 -seed 3
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 4
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Inv == x < 3
Live == <>(x = 7)
Post == PrintT(CounterExample)
Alias == [v |-> x]
====
SPECIFICATION Spec
INVARIANT Inv
POSTCONDITION Post
ALIAS Alias
