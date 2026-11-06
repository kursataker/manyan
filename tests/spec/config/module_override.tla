---- MODULE module_override ----
\* A definition of a module the spec uses, overridden for it alone.
EXTENDS Naturals, TLC
L == INSTANCE Lib
Small == 3
Triple(x) == 3 * x
ASSUME PrintT(<<L!Limit, L!Double(2)>>)
====
CONSTANT Limit <- [Lib] Small
CONSTANT Double <- [Lib] Triple
