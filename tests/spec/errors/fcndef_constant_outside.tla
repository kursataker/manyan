---- MODULE fcndef_constant_outside ----
\* A function definition that is not recursive is computed in full before
\* checking, so applying it outside its domain names its value.
EXTENDS Integers, TLC
f[x \in 1..2] == x * 10
ASSUME PrintT(f[3])
====
