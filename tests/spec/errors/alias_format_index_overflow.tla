---- MODULE alias_format_index_overflow ----
\* An alias's _format whose argument index is more than an int holds is
\* Java's exception, ending the behavior.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Inv == x < 2
Alias == [_format |-> "%4294967295$s", x |-> x]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
