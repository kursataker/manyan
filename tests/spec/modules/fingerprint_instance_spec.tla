---- MODULE fingerprint_instance_spec ----
\* Through a named instance's specification: TLC splits I!Spec and enters
\* the instance, so the stack has no frame of the call I!Spec.
VARIABLE w
I == INSTANCE FPM WITH v <- w
Spec == I!Spec
====
SPECIFICATION Spec
