---- MODULE subset_overflow ----
\* TLC counts SUBSET S before enumerating it: S of 31 elements or more
\* overflows, and each SUBSET with a source is in the fingerprint stack
\* (TLC's FingerprintExceptionInit).
EXTENDS Integers
VARIABLE x
Init == x = SUBSET(SUBSET(1..36))
Next == x' = x
====
INIT Init
NEXT Next
