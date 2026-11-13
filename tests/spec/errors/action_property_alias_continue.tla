---- MODULE action_property_alias_continue ----
\* options: -continue
\* With -continue an alias failing to print the behavior fails the action
\* property, and the step; TLC checks the step again as it replays it.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 2 /\ x' = x + 1
Prop == [][x' < 2]_x
Alias == [_format |-> "%X=%s", x |-> x]
====
INIT Init
NEXT Next
PROPERTY Prop
ALIAS Alias
