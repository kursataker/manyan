---- MODULE functions ----
EXTENDS Naturals, TLC
f == [x \in 1..3 |-> x * 10]
g == [x \in {"a", "b"}, y \in 1..2 |-> y]
h == [x \in {0, 2} |-> x]
ASSUME PrintT(<<f, f[2], DOMAIN f, g["a", 2], g[<<"b", 1>>], DOMAIN g, h>>)
ASSUME PrintT(<<[{1, 2} -> {"x", "y"}], [{} -> {1}], [x \in {} |-> x]>>)
ASSUME PrintT(<<f = <<10, 20, 30>>, h = (0 :> 0 @@ 2 :> 2)>>)
====
