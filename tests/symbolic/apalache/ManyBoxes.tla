\* options: -length 10
\* apalache-mc check --temporal=Liveness ManyBoxes.tla: OK
---- MODULE ManyBoxes ----

\* Tests behaviour when there are 
\* superfluous box operators in the formula

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

Liveness == [][][][][][]<>(x = 1)
FalseLiveness == [][][][][][]<>(x = 2)

====
INIT Init
NEXT Next
PROPERTY Liveness
