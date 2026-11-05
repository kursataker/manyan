---- MODULE sets ----
EXTENDS Naturals, TLC
ASSUME PrintT(<<{3, 1, 2, 1}, {}, {{2}, {1, 2}, {}}, 2 \in {1, 2}, 3 \notin {1, 2}>>)
ASSUME PrintT(<<{1, 2} \cup {2, 3}, {1, 2} \union {5}, {1, 2} \cap {2, 3}, {1, 2} \intersect {}, {1, 2, 3} \ {2}>>)
ASSUME PrintT(<<{1} \subseteq {1, 2}, {1, 3} \subseteq {1, 2}, {} \subseteq {}>>)
ASSUME PrintT(<<SUBSET {1, 2}, UNION {{1}, {2, 3}, {}}, SUBSET {}>>)
ASSUME PrintT(<<{x \in 1..10 : x % 3 = 0}, {x * x : x \in 1..4}, {x + y : x \in 1..2, y \in {10, 20}}>>)
ASSUME PrintT(<<{<<x, y>> \in {1, 2} \X {3} : x = 2}, {a + b : <<a, b>> \in {<<1, 2>>, <<3, 4>>}}>>)
ASSUME PrintT(<<1..3, 3..1, {1, 2} = {2, 1}, {1} = {1, 2}>>)
====
