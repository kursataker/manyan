---- MODULE fingerprint_successor_again ----
\* TLC finds the unfingerprintable successor again, comparing SUBSETs by
\* their bases, and fails again on it (TLC's FingerprintExceptionNext).
EXTENDS Integers
VARIABLE x
Init == x = 0
Next == IF {1} \in SUBSET(1..31) THEN x' = SUBSET(SUBSET(1..32)) ELSE x' = 2
====
INIT Init
NEXT Next
