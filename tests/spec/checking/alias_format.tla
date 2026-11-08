---- MODULE alias_format ----
\* An alias's field _format is the Java format of each other field's name
\* and value: flags, widths, precisions, argument indices, %S, %b, %h, %n
\* and %%.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Inv == x < 2
Alias == [_format |-> "[%-6s|%5.3S|%2$s=%1$s|%2$b|%1$h|%%]%n", x |-> x, y |-> <<x, "ab">>]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
