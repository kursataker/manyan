---- MODULE counter_example_lasso ----
\* A simulation's lasso.
\* options: -simulate num=3 -depth 6 -seed 3
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Live == <>(x = 7)
Post == PrintT(CounterExample)
====
SPECIFICATION Spec
PROPERTY Live
POSTCONDITION Post
