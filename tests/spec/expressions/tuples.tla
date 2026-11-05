---- MODULE tuples ----
EXTENDS Naturals, TLC
t == <<1, "two", {3}>>
ASSUME PrintT(<<t, t[2], DOMAIN t, <<>>, <<<<1>>, <<>> >> >>)
ASSUME PrintT(<<{1, 2} \X {"a"}, {1} \X {2} \X {3}, ({1} \X {2}) \X {3}>>)
ASSUME PrintT(<<<<1, 2>> = [i \in 1..2 |-> i], <<>> = [x \in {} |-> 0]>>)
====
