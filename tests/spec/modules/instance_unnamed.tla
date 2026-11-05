---- MODULE instance_unnamed ----
EXTENDS TLC
Max == 7
INSTANCE Shapes
ASSUME PrintT(<<Square(Max), Area>>)
====
