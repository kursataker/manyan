---- MODULE json_array_record ----
\* ToJsonArray's argument a record is an error of its Java.
EXTENDS Json, TLC, Naturals
ASSUME PrintT(ToJsonArray([a |-> 1]))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
