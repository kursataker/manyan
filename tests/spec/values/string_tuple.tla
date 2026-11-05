---- MODULE string_tuple ----
\* TLC compares a tuple with a string as with the tuple of its characters,
\* and finds no value in an empty interval.
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<<<"a">> # "-", <<"-">> = "-", <<>> = "", <<"a", "b">> = "ab", <<"ab">> = "ab", Tail(<<1, "b">>) = "b">>)
ASSUME PrintT(<<"m" \in 1..0, "m" \notin 1..0, <<1>> \in 2..1, "m" \in DOMAIN <<>>>>)
ASSUME PrintT([i \in {1} |-> "a"] = "a")
====
