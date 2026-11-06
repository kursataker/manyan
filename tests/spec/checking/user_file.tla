---- MODULE user_file ----
\* -userFile writes what Print and PrintT print to the file, the output of
\* a rebuilt trace too, and the report to standard output.
\* options: -userFile user.txt
EXTENDS Naturals, TLC
VARIABLE x
ASSUME PrintT("assumed")
Init == x = 0
Next == Print(x, x < 3) /\ x' = x + 1
I == x < 3
====
INIT Init
NEXT Next
INVARIANT I
