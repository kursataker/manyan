---- MODULE alias_format_conversion_continue ----
\* options: -continue
\* With -continue TLC prints the behavior inside its check of the invariant:
\* the alias failing to print it fails the invariant, and then the step.
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
