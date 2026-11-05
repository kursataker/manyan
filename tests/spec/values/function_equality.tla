---- MODULE function_equality ----
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<<<"a">> = [i \in {1} |-> "a"], [a |-> 1] = [x \in {"a"} |-> 1], [x \in {} |-> 1] = <<>>>>)
ASSUME PrintT(<<{<<1>>, [i \in 1..1 |-> 1]}, Append(<<>>, 1) = <<1>> >>)
====
