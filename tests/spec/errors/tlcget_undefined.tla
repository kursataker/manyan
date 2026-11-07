---- MODULE tlcget_undefined ----
\* TLCGet of a name it does not know.
EXTENDS TLC, Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
ASSUME TLCGet("mode") = "bfs"
====
INIT Init
NEXT Next
