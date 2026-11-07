---- MODULE tlcget_property ----
\* TLCGet("-Dk") is Java's system property k, or "-Dk" when it is unset;
\* TLCSet("-Dk", v) sets it to v as TLC prints it. Found by
\* tlaplus/Examples' MCCRDT, which asks for -Dtlc2.tool.impl.Tool.cdot.
EXTENDS Naturals, TLC
VARIABLE x
ASSUME PrintT(TLCGet("-Dtlc2.tool.impl.Tool.cdot"))
ASSUME TLCSet("-Dtpp.key", "v") /\ PrintT(TLCGet("-Dtpp.key"))
ASSUME TLCSet("-Dtpp.key", <<1, {2}>>) /\ PrintT(TLCGet("-Dtpp.key"))
Init == x = 0
Next == x < 2 /\ x' = x + 1 /\ TLCSet("-Dtpp.x", x) /\ PrintT(TLCGet("-Dtpp.x"))
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
