---- MODULE counter_example_stuttering_initial ----
\* A lasso stuttering at its initial state steps by an action TLC does not
\* know.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x
Live == <>(x = 1)
Post == PrintT(CounterExample)
====
SPECIFICATION Spec
PROPERTY Live
POSTCONDITION Post
