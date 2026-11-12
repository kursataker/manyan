---- MODULE counter_example_lasso ----
\* A lasso's CounterExample steps from its last state back to the state
\* the loop starts at, by the action that closes it.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Live == <>(x = 5)
Post == PrintT(CounterExample)
Alias == [z |-> x]
====
SPECIFICATION Spec
PROPERTY Live
POSTCONDITION Post
ALIAS Alias
