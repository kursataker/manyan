---- MODULE string_backslash_at_end ----
\* A module that ends inside a string, right after a backslash, is a parse error.
EXTENDS Ends
Next == x' = x
====
INIT Init
NEXT Next
