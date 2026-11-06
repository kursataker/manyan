---- MODULE standard_name ----
\* A module the user supplies with a standard module's name has TLC's Java
\* operators where the names match (Starydark/PaxosStore-tla ships its own
\* FiniteSets, whose Cardinality TLC cannot evaluate); others are its own.
EXTENDS FiniteSets, TLC
ASSUME PrintT(<<Cardinality({1, 2}), Extra({})>>)
====
