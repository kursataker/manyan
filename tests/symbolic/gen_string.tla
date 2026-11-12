\* options: -length 2
---- MODULE gen_string ----
\* Gen of a string, or of an uninterpreted type: one the specification
\* has, or a fresh one.
EXTENDS Apalache
VARIABLES
    \* @type: Str;
    x,
    \* @type: Str;
    y,
    \* @type: T;
    u,
    \* @type: T;
    w
Init == x = Gen(1) /\ y = "b" /\ u = Gen(1) /\ w = "t1_OF_T"
Next == x' = Gen(1) /\ y' = x /\ u' = Gen(1) /\ UNCHANGED w
Inv2 == ~(y \notin {"a", "b"} /\ u = w)
====
INIT Init
NEXT Next
INVARIANT Inv2
