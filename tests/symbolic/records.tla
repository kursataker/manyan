---- MODULE records ----
\* A record's fields change one at a time.
EXTENDS Integers
VARIABLES
    \* @type: { count: Int, name: Str };
    r
Init == r = [count |-> 0, name |-> "a"]
Next == \/ r' = [r EXCEPT !.count = @ + 1]
        \/ r' = [r EXCEPT !.name = IF @ = "a" THEN "b" ELSE "a"]
Inv == ~(r.count = 3 /\ r.name = "b")
====
INIT Init
NEXT Next
INVARIANT Inv
