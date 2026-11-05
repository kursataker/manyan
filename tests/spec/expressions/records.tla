---- MODULE records ----
EXTENDS Naturals, TLC
r == [name |-> "n", age |-> 3]
ASSUME PrintT(<<r, r.name, r["age"], DOMAIN r, [age |-> 3, name |-> "n"] = r>>)
ASSUME PrintT([kind : {"k1", "k2"}, val : 1..2])
ASSUME PrintT([x \in {"age", "name"} |-> 0])
====
