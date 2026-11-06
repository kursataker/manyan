---- MODULE module_value ----
\* A definition of a module the spec uses, given a value for it alone.
EXTENDS TLC
CONSTANT M
L == INSTANCE Lib
None == 0
ASSUME PrintT(<<L!None, None>>)
====
CONSTANT M = M
CONSTANT None = [Lib] M
