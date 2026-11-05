---- MODULE constants ----
EXTENDS Naturals, TLC
CONSTANTS N, Name, Flag, Items, Def, Op(_)
MyDef == N * 100
MyOp(x) == x + N
ASSUME PrintT(<<N, Name, Flag, Items, Def, Op(1)>>)
====
CONSTANT N = 3
CONSTANT Name = "tpp"
CONSTANTS
    Flag = FALSE
    Items = {m, n}
    Def <- MyDef
    Op <- MyOp
