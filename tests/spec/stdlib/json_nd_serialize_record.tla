---- MODULE json_nd_serialize_record ----
\* ndJsonSerialize takes a sequence.
EXTENDS Json, TLC, Naturals
ASSUME ndJsonSerialize("out.ndjson", [a |-> 1])
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
