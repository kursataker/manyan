\* options: -length 10
\* apalache-mc check --config=ConfigParams.cfg ConfigParams.tla: OK
--------------------------- MODULE ConfigParams -------------------------------
CONSTANTS
  \* @type: Int;
  MyInt,
  \* @type: Str;
  MyStr,
  \* TODO: use a model type here
  \* when #570 is closed: https://github.com/apalache-mc/apalache/issues/570
  \* @type: Str;
  MyModelValue1,
  \* @type: Str;
  MyModelValue2,
  \* @type: Set(Int);
  MySet

VARIABLES
  \* @type: Int;
  x,
  \* @type: Str;
  y,
  \* @type: Str;
  z,
  \* @type: Set(Int);
  w

Init ==
    /\ x = MyInt
    /\ y = MyStr
    /\ z = MyModelValue1
    /\ w = MySet

Next ==
    UNCHANGED <<x, y, z, w>>

Inv ==
    /\ x = 42
    /\ y = "hello"
    /\ z = MyModelValue1
    /\ z /= MyModelValue2
    /\ w = { 1, 2, 3 }

===============================================================================
INIT Init
NEXT Next
INVARIANT Inv
CONSTANTS MyInt = 42
MyStr = "hello"
MyModelValue1 = Model1
MyModelValue2 = Model2
MySet = {1, 2, 3}
