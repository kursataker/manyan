---- MODULE tuple_source ----
\* An invariant of an initial state applies a tuple made by <<...>>
\* outside its domain: TLC cannot fingerprint the tuple, which it names.
EXTENDS Naturals
VARIABLE t
Init == t = <<1, 2>>
Inv == t[3] = 0
Next == UNCHANGED t
====
INIT Init
NEXT Next
INVARIANT Inv
