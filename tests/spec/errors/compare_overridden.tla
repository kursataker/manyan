---- MODULE compare_overridden ----
\* Two of TLC's overridden sets compare by the first one's module, whose
\* error prints no initial state: ANY with anything, Nat with ANY (tlx's
\* SA21).
EXTENDS TLC, Naturals
VARIABLE x
Init == x = 0
Next == x' = x
Inv == Nat = Any
====
INIT Init
NEXT Next
INVARIANT Inv
