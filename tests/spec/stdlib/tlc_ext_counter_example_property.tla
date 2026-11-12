---- MODULE tlc_ext_counter_example_property ----
\* A property violated by an initial state.
\* options: -nowarning
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x \in {0, 1}
Next == x' = x + 1
Spec == Init /\ [][Next]_x
InitProp == x = 0
Post == PrintT(CounterExample)
====
SPECIFICATION Spec
PROPERTY InitProp
POSTCONDITION Post
