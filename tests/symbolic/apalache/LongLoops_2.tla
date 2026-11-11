\* options: -length 10
\* apalache-mc check --temporal=FalseLiveness LongLoops.tla: ERROR (12)
---- MODULE LongLoops_2 ----

\* Tests behaviour when loops are long (must have more than one state)

EXTENDS Integers

VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y

Range == {value \in Int: value >= 0 /\ value <= 50}

Init ==
    /\ x \in Range
    /\ y  \in Range
    /\ x # y

Next ==
    /\ x' = y
    /\ y' = x

Liveness == [](x = 0 => <>(x # 0 /\ y = 0))
FalseLiveness == [](x = 0 => <>(x = 0 /\ y = 0))

====
INIT Init
NEXT Next
PROPERTY FalseLiveness
