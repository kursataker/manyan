---- MODULE temporal_action ----
\* A temporal formula cannot contain a raw action.
VARIABLE x
Init == x = 0
Next == x' = x
Prop == [](x' = x)
====
INIT Init
NEXT Next
