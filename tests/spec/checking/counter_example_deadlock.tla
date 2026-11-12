---- MODULE counter_example_deadlock ----
\* A deadlock's CounterExample ends at the state deadlocked; an action
\* property's at the step that violates it.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Post == PrintT(ToTrace(CounterExample))
====
INIT Init
NEXT Next
POSTCONDITION Post
