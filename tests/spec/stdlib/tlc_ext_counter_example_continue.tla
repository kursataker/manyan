---- MODULE tlc_ext_counter_example_continue ----
\* options: -continue
\* With -continue TLC sets up no trace spec, which interns "state" first:
\* a CounterExample is [action, state].
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 1 /\ x' = x + 1
Inv == PrintT(CounterExample)
====
INIT Init
NEXT Next
INVARIANT Inv
CHECK_DEADLOCK FALSE
