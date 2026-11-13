---- MODULE json_record_order_simulate ----
\* options: -simulate num=1 -depth 1 -seed 1
\* A simulation checks the initial state's invariants as the search does:
\* a record made there is written as it was written, not in TLC's order.
EXTENDS Json, TLC
VARIABLE x
Init == x = 0
Next == UNCHANGED x
Inv == PrintT(ToJson([b |-> 1, a |-> 2, zz |-> 3, c |-> 4]))
====
INIT Init
NEXT Next
INVARIANT Inv
