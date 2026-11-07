---- MODULE substitute_java ----
\* A constant substitution that fails in a Java operator is reported as
\* the initial states are computed.
EXTENDS Naturals
CONSTANT C
VARIABLE x
Foo == 1 + "a"
Init == x = C
Next == x' = x
====
CONSTANT C <- Foo
INIT Init
NEXT Next
