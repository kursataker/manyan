---- MODULE fingerprint_instance_call ----
\* An initial predicate that calls I!Init: the call's frame, then the
\* instance's.
VARIABLE w
I == INSTANCE FPM WITH v <- w
Init == I!Init
Next == I!Next
====
INIT Init
NEXT Next
