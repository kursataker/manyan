---- MODULE java_error ----
\* An error inside an operator TLC implements in Java is a
\* TLCRuntimeException: the simulator reports it plainly.
\* options: -simulate -depth 3 -seed 2
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 + "a"
====
INIT Init
NEXT Next
