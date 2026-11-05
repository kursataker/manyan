---- MODULE integer_too_big ----
\* TLC rejects an integer literal above 2^31 - 1 before checking, even unused.
EXTENDS Integers, FiniteSets, TLC
Unused == 3000000000
====
