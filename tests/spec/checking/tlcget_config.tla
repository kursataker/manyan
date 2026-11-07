---- MODULE tlcget_config ----
\* TLCGet("config") describes the search, in an assumption too
\* (DieHard/MCDieHardest from tlaplus/Examples).
\* options: -seed 42 -deadlock
EXTENDS TLC, Naturals
VARIABLE x
C == TLCGet("config")
ASSUME PrintT(<<C.mode, C.deadlock, C.worker, C.seed>>)
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
