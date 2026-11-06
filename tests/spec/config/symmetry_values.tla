---- MODULE symmetry_values ----
\* SYMMETRY names a set of permutations: functions.
VARIABLE x
Init == x = 0
Perms == {1, 2}
====
INIT Init
NEXT Init
SYMMETRY Perms
