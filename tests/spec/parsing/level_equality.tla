---- MODULE level_equality ----
\* Most built-in operators take at most actions: a temporal formula may
\* not be compared, here through a function whose values are temporal.
VARIABLE x
F == [y \in {1} |-> [](x = 0)]
Same == F = F
====
INIT Same
NEXT Same
