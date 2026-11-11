---- MODULE json_object_set ----
\* ToJsonObject's argument a set is an error of its Java.
EXTENDS Json, TLC, Naturals
ASSUME PrintT(ToJsonObject({5, 6}))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
