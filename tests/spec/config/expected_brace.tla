---- MODULE expected_brace ----
\* A set's elements are separated by commas; -1 is not a value, as the
\* configuration has no negative numbers.
CONSTANT S
VARIABLE x
Init == x \in S
====
CONSTANT S = {1 -1}
INIT Init
NEXT Init
