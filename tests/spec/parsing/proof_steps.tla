---- MODULE proof_steps ----
\* A theorem's statement ends where its proof's first step begins, though
\* <1>1 could otherwise continue it as a comparison.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x' = x
Spec == Init /\ [][Next]_x
THEOREM Spec => []Init
<1>1. Init => Init
  OBVIOUS
<1>2. QED
  BY <1>1
ASSUME PrintT("checked")
====
INIT Init
NEXT Next
