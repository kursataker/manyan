---- MODULE undefined_primed ----
\* TLC's simulator reports a TLCRuntimeException, such as an undefined
\* primed variable, plainly, with the status of its error code.
\* options: -simulate -depth 3 -seed 2
VARIABLE x
Init == x = 0
Next == x' = x'
====
INIT Init
NEXT Next
