---- MODULE json_missing ----
\* A file ndJsonDeserialize cannot read is an error of its Java.
EXTENDS Json, TLC
ASSUME PrintT(ndJsonDeserialize("missing.ndjson"))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
