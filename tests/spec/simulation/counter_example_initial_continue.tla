---- MODULE counter_example_initial_continue ----
\* options: -simulate num=1 -depth 7 -seed 381 -continue
\* With -continue, TLC's simulator still gives an initial state that
\* violates an invariant as a CounterExample of one state.
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
