---- MODULE undefined_symbol ----
\* \mod is not a TLA+ operator symbol, so it cannot be defined.
EXTENDS Naturals
VARIABLE x
u \mod v == u % v
Init == x = 0
Next == x' = x
====
INIT Init
NEXT Next
