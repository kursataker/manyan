---- MODULE counter_example_alias ----
\* With an ALIAS, the states of a CounterExample are the ALIAS's, as TLC
\* evaluates it on each step; one it fails on keeps its variables and the
\* error.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Init == x = 0
Next == x < 4 /\ x' = x + 1
Inv == x # 2
Post == PrintT(ToTrace(CounterExample))
Alias == [y |-> 10 \div (2 - x), next |-> x']
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
ALIAS Alias
