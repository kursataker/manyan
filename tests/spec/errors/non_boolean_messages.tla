---- MODULE non_boolean_messages ----
\* TLC's message for each place a boolean was expected: a quantifier's or
\* CHOOSE's body names the whole expression, a CASE condition itself, a
\* filter what its predicate was; ~ names the kind. A record applied to a
\* non-string, an interval joined with a string, and a tuple pattern
\* matched with a non-tuple fail as TLC does.
EXTENDS Integers, TLC, TLCExt
ASSUME AssertError("TLC expected a boolean value, but did not find one. line 8, col 141 to line 8, col 159 of module non_boolean_messages", \A x \in {1, 2} : x)
ASSUME AssertError("TLC expected a boolean value, but did not find one. line 9, col 141 to line 9, col 163 of module non_boolean_messages", CHOOSE x \in {1, 2} : x)
ASSUME AssertError("A non-boolean expression (an integer) was used as a condition of a CASE. line 10, col 169 to line 10, col 173 of module non_boolean_messages", CASE 1 + 1 -> 2)
ASSUME AssertError("Attempted to access record by a non-string argument: 1", [a |-> 1][1])
ASSUME AssertError("Attempted to check if the value:\n\"a\"\nis in the integer interval 1..3", (1 .. 3) \cup {"a"} = {})
ASSUME AssertError("Argument mismatch in operator application.%1", \E <<a, b>> \in {1} : TRUE)
====
