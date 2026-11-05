---- MODULE model_values ----
EXTENDS TLC
CONSTANTS Procs, NoVal, Val
ASSUME PrintT(<<Procs, NoVal, NoVal = NoVal, NoVal \in Procs, Val, NoVal = 1, NoVal = "s">>)
ASSUME PrintT([p \in Procs |-> p])
====
CONSTANTS
  Procs = {p3, p1, p2}
  NoVal = NoVal
  Val = 42
