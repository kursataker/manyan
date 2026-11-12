---- MODULE tlc_ext_to_trace_error ----
\* ToTrace takes a CounterExample, not a record of its fields.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Post == PrintT(ToTrace([state |-> {}, action |-> {}]))
====
INIT Init
NEXT Next
POSTCONDITION Post
CHECK_DEADLOCK FALSE
