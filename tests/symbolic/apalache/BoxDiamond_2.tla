\* options: -length 10
\* apalache-mc check --temporal=FalseLiveness BoxDiamond.tla: ERROR (12)
---- MODULE BoxDiamond_2 ----

\* Tests behaviour when a box operator is followed by a diamond operator

EXTENDS Integers

VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y

Init ==
    /\ x = 0
    /\ y = 1

Next ==
    /\ x' = y
    /\ y' = x

Liveness == []<>(x = 1 /\ y = 0)
FalseLiveness == []<>(x = 0 /\ y = 0)

====
INIT Init
NEXT Next
PROPERTY FalseLiveness
