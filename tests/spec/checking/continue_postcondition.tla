---- MODULE continue_postcondition ----
\* With -continue, TLC checks no postcondition after an invariant it
\* reports violated: only after the deadlock that stops it.
\* options: -continue
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 4 /\ x' = x + 1
Inv == x # 2
Post == PrintT(<<"post", ToTrace(CounterExample)>>)
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
