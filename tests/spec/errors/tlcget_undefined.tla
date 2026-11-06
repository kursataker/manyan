---- MODULE tlcget_undefined ----
\* TLC 2.19 knows no TLCGet("config") (DieHard/MCDieHardest from
\* tlaplus/Examples).
EXTENDS TLC, Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
ASSUME TLCGet("config").mode = "bfs"
====
INIT Init
NEXT Next
