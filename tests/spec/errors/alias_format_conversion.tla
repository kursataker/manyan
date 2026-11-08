---- MODULE alias_format_conversion ----
\* A format of an alias's _format that Java cannot apply to the strings
\* of a field's name and value is Java's exception, ending the behavior.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Inv == x < 2
Alias == [_format |-> "%X=%s", x |-> x]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
