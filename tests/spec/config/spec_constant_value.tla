---- MODULE spec_constant_value ----
\* A conjunct of a specification that is a constant other than a boolean,
\* even a literal TLC does not compute before, cannot be one.
VARIABLE x
Init == "a"
Next == x' = 1
Spec == Init /\ [][Next]_x
====
SPECIFICATION Spec
