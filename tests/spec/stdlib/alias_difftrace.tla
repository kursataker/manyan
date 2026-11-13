---- MODULE alias_difftrace ----
\* options: -difftrace
\* With -difftrace TLC leaves out the fields of an alias it shows unchanged,
\* but in the last state.
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x' = x + 1 /\ y' = y
Alias == [x |-> x, y |-> y, sum |-> x + y, c |-> 7, nx |-> x']
Small == x < 3
====
INIT Init
NEXT Next
ALIAS Alias
INVARIANT Small
