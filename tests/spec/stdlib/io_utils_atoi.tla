---- MODULE io_utils_atoi ----
\* atoi of a string that is no number is an error, as in TLC's Java.
EXTENDS IOUtils, TLC
ASSUME PrintT(atoi("12x"))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
