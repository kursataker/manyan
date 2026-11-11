\* options: -length 10 -cinit CInit
\* apalache-mc check --cinit=CInit --inv=Inv ModelValFail.tla: ERROR (120)
---------- MODULE ModelValFail ----------

EXTENDS Integers

CONSTANT 
  \* @type: UT;
  C,
  \* @type: XY;
  G

VARIABLE
  \* @type: Set(UT);
  x,
  \* @type: Set(XY);
  z,
  \* @type: Str;
  s

CInit == C = "uval_UT_1" /\ G = "uval_XY_1"

Init == x = {} /\ z = {G} /\ s = "init"

Next == x' = {C} /\ z' = {} /\ s' = "next"

\* Should fail, can't compare model values of different types
Inv == C = G
====================
INIT Init
NEXT Next
INVARIANT Inv
