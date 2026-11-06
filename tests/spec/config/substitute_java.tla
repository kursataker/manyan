---- MODULE substitute_java ----
\* A substitution that fails in a Java operator names the error.
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
