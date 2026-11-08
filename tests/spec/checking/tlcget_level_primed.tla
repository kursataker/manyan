---- MODULE tlcget_level_primed ----
\* TLCGet("level")' is the level of the next state.
EXTENDS Naturals, TLC
VARIABLE x, z
Init == x = 0 /\ z = 1
Next == /\ x < 3
        /\ Assert(TLCGet("level") + 1 = TLCGet("level")', "level'")
        /\ x' = x + 1
        /\ z' = TLCGet("level")'
Inv == z = TLCGet("level")
Prop == [][TLCGet("level") + 1 = TLCGet("level")']_x
====
INIT Init
NEXT Next
INVARIANT Inv
PROPERTY Prop
CHECK_DEADLOCK FALSE
