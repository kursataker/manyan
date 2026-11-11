---- MODULE strings ----
\* Strings compare by their characters only.
VARIABLES
    \* @type: Str;
    s
Init == s \in {"a", "b"}
Next == s' = IF s = "a" THEN "c" ELSE "a"
Inv == s /= "c"
====
INIT Init
NEXT Next
INVARIANT Inv
