---- MODULE state_holds_nat ----
\* A successor holds Nat inside a set: TLC cannot fingerprint the state.
EXTENDS Naturals
VARIABLE x
Init == x = {}
Next == x' = {Nat}
====
INIT Init
NEXT Next
