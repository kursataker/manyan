---- MODULE control ----
\* IF, CASE and LET in actions and in values.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x,
    \* @type: Str;
    mode
Init == x = 0 /\ mode = "up"
Next == /\ IF mode = "up" THEN x' = x + 2 ELSE x' = x - 1
        /\ mode' = CASE x' > 5 -> "down" [] x' < 1 -> "up" [] OTHER -> mode
Inv == LET low == 0 high == 7 IN x >= low /\ x < high
====
INIT Init
NEXT Next
INVARIANT Inv
