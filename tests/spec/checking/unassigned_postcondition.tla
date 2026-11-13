---- MODULE unassigned_postcondition ----
\* A successor that leaves a variable out: TLC fails looking it up for the
\* postconditions, and checks none.
EXTENDS TLC, TLCExt
VARIABLES x, y
Init == x = 1 /\ y = 1
Next == x' = 2
Post == PrintT(CounterExample)
====
INIT Init
NEXT Next
POSTCONDITION Post
