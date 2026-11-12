---- MODULE tlc_ext_counter_example_initial ----
\* An initial state that violates an invariant, or a property, is a
\* CounterExample of one state and no step.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x \in {0, 5}
Next == x' = x + 1
Spec == Init /\ [][Next]_x
Inv == x < 5
Post == PrintT(CounterExample) /\ PrintT(ToTrace(CounterExample))
====
SPECIFICATION Spec
INVARIANT Inv
POSTCONDITION Post
