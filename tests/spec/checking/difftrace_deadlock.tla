---- MODULE difftrace_deadlock ----
\* -difftrace with one variable: the deadlocked state, the last, is
\* printed whole; the others show what changed.
\* options: -difftrace
EXTENDS Naturals, Sequences
VARIABLE x
Init == x = <<0>>
Next == Len(x) < 3 /\ x' = Append(x, Len(x))
====
INIT Init
NEXT Next
