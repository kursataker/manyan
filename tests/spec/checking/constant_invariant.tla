---- MODULE constant_invariant ----
\* TLC evaluates a constant invariant as it reads the configuration: one
\* that is TRUE is dropped, another value is an error.
EXTENDS Naturals
CONSTANT N
VARIABLE x
Init == x = 0
Next == x' = x
Small == N < 3
Big == N > 3
====
CONSTANT N = 5
INIT Init
NEXT Next
INVARIANT Big Small
