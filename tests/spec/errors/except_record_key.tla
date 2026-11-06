---- MODULE except_record_key ----
\* EXCEPT on a record at a key that is not a string is TLC's error.
EXTENDS TLC
ASSUME PrintT([[a |-> 1] EXCEPT ![3] = 2])
====
