---- MODULE unannotated_order ----
\* Without annotations, Apalache names a variable before any constant, the
\* last in the order SANY's table of the module's names gives them, not in
\* the order they are declared.
CONSTANT Alpha, Beta
VARIABLE uState, uIP, AuthChannel, FwState, aIP, sChannel
Init == uState = 0
====
