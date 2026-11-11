---- MODULE json_unsupported ----
\* A value Gson cannot write, an infinite set, is an error of Json's Java.
EXTENDS Json, TLC, Naturals
ASSUME PrintT(ToJson(Nat))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
