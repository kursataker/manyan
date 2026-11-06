---- MODULE tlcget_argument ----
\* TLCGet of a value that is neither a string nor a nonnegative integer.
EXTENDS TLC, Integers
VARIABLE x
Init == x = 0
Next == UNCHANGED x
ASSUME TLCGet(-1) = 0
====
INIT Init
NEXT Next
