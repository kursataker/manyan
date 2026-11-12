---- MODULE counter_example_safety ----
\* A temporal property TLC finds violated as a safety property: the
\* behavior to the state that violates it, its states the ALIAS's.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x \in {0, 1}
Next == x < 4 /\ x' = x + 1
Spec == Init /\ [][Next]_x
Safe == [](x < 3)
Post == PrintT(CounterExample)
Alias == [w |-> x]
====
SPECIFICATION Spec
PROPERTY Safe
POSTCONDITION Post
ALIAS Alias
