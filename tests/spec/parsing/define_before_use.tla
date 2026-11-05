---- MODULE define_before_use ----
\* A definition may only use names defined above it.
VARIABLE x
Init == x = Zero
Zero == 0
Next == x' = x
====
INIT Init
NEXT Next
