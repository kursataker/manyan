---- MODULE monolith ----
\* A module in no file of its own is read from the root module's file,
\* from its MODULE line to the next ====, numbered from that line, as
\* SANY reads a monolithic spec. Found by tlaplus/Examples' braf.
EXTENDS Naturals, Common
VARIABLE x
I == INSTANCE Counter WITH c <- x
Init == I!Init
Next == I!Next
Inv == x < Limit + Bad(x)
====

Text between the modules is not read.

------------------------------ MODULE Counter ------------------------------
EXTENDS Naturals
VARIABLE c
Init == c = 0
Next == c' = c + 1 /\ c < 5
=============================================================================

---- MODULE Common ----
EXTENDS Naturals
Bad(n) == IF n < 4 THEN 0 ELSE CHOOSE y \in {} : TRUE
Limit == 10
====
INIT Init
NEXT Next
INVARIANT Inv
