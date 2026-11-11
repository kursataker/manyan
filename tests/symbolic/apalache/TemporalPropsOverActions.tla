\* options: -length 10
\* apalache-mc check --temporal=Liveness TemporalPropsOverActions.tla: OK
---- MODULE TemporalPropsOverActions ----

EXTENDS Integers

VARIABLE
    \* @type: Int;
    x

Init ==
    x = 0

Next ==
    x' = IF x < 3 THEN x + 1 ELSE 0
    
Liveness ==
    <>(<<x' = x + 1>>_x)

FalseLiveness ==
    <>[]([x' = x + 1]_x)
====
INIT Init
NEXT Next
PROPERTY Liveness
