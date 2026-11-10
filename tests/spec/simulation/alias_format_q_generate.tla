---- MODULE alias_format_q_generate ----
\* A format of an alias's _format that Java cannot apply is Java's exception,
\* ending the behavior; a simulation prints the behavior once, before it.
\* options: -generate -depth 5 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1
Inv == x < 2
Alias == [_format |-> "%q", x |-> x]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
