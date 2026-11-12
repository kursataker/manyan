---- MODULE tlc_ext_counter_example_none ----
\* Without an error, and outside a postcondition, CounterExample has no
\* state and no step.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Inv == PrintT(CounterExample)
Post == PrintT(<<CounterExample, ToTrace(CounterExample)>>)
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
CHECK_DEADLOCK FALSE
