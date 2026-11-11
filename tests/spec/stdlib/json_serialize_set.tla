---- MODULE json_serialize_set ----
\* JsonSerialize takes a sequence or a record.
EXTENDS Json, TLC, Naturals
ASSUME JsonSerialize("out.json", {1})
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
