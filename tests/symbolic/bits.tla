---- MODULE bits ----
\* A boolean function over a set; \A and \E of a symbolic one.
VARIABLES
    \* @type: Int -> Bool;
    on
Init == on = [i \in {1, 2, 3} |-> FALSE]
Next == \E i \in DOMAIN on : on' = [on EXCEPT ![i] = ~@]
AllOff == \E i \in DOMAIN on : ~on[i]
====
INIT Init
NEXT Next
INVARIANT AllOff
