---- MODULE invariant_not_state ----
\* An invariant must be a state predicate; TLC says so before it looks at
\* the other names the configuration gives, such as an undefined property.
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == x' = x + 1 /\ x < 3
Spec == Init /\ [][Next]_x
Positive == x > 0
AlwaysPositive == [](x > 0)
====
SPECIFICATION Spec
PROPERTY Undefined
INVARIANT Positive AlwaysPositive
