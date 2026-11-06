---- MODULE except_tuple_key ----
\* EXCEPT on a tuple at a key that is not an integer is TLC's error, not
\* the warning for a missing key (tlx's EmptyMapExcept).
EXTENDS Naturals, TLC
VARIABLE rec
Init == rec = ("m" :> 0)
Drop == rec' = <<>>
Update == rec' = [rec EXCEPT !["m"] = 2]
Next == Drop \/ Update
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
