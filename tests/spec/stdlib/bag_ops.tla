---- MODULE bag_ops ----
EXTENDS Naturals, Bags, TLC
B1 == SetToBag({"a", "b"})
B2 == ("a" :> 2)
ASSUME PrintT(<<B1, B1 (+) B2, (B1 (+) B2) (-) B1, BagToSet(B2), BagIn("a", B2), BagIn("c", B1), CopiesIn("a", B1 (+) B2)>>)
ASSUME PrintT(<<IsABag(B1), BagCardinality(B1 (+) B2), EmptyBag, B2 \sqsubseteq (B1 (+) B2), BagUnion({B1, B2})>>)
ASSUME PrintT(BagOfAll(LAMBDA x : x % 2, SetToBag({1, 2, 3})))
====
