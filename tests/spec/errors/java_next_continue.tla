---- MODULE java_next_continue ----
\* options: -continue
\* With -continue TLC loads no Integers for a trace spec, and names the Java
\* method of Naturals; it goes on to find the state deadlocked.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = 1 + "a"
====
INIT Init
NEXT Next
