---- MODULE tab_columns ----
\* A tab moves to the next column after a multiple of 8, as in SANY: the
\* junction below, one line indented by spaces and one by a tab, is aligned.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next ==
        /\ x < 2
	/\ x' = x + 1
Inv ==
		x < 1
====
INIT Init
NEXT Next
INVARIANT Inv
