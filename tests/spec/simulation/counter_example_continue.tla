---- MODULE counter_example_continue ----
\* options: -simulate num=1 -depth 5 -seed 1 -continue
\* With -continue, TLC's simulator keeps no trace for a postcondition:
\* CounterExample is empty.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 4 /\ x' = x + 1
Inv == x # 2
Post == PrintT(CounterExample)
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
