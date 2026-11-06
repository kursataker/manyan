---- MODULE unassigned_order ----
\* Of several constants without a value, TLC names the first in the order
\* of SANY's hash table of the module's symbols (ceph from
\* afonsonf/ceph-consensus-spec).
EXTENDS Naturals
CONSTANTS Delta, Epsilon, Beta, Gamma, Alpha
VARIABLE x
Init == x = 0
Next == x' = x
====
INIT Init
NEXT Next
