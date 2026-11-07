---- MODULE string_tuple ----
\* TLC finds no value in an empty interval, and compares no tuple with a
\* string.
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<"m" \in 1..0, "m" \notin 1..0, <<1>> \in 2..1, "m" \in DOMAIN <<>>>>)
ASSUME PrintT(<<"a", "b">> = "ab")
====
