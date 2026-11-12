---- MODULE enabled_fair ----
\* Flip is enabled infinitely often with b true, in every fair behavior.
EXTENDS Naturals
VARIABLES
    \* @type: Int;
    x,
    \* @type: Bool;
    b
\* @type: <<Int, Bool>>;
vars == <<x, b>>
Init == x = 0 /\ b = FALSE
Inc == x < 3 /\ x' = x + 1 /\ UNCHANGED b
Flip == x = 3 /\ b' = ~b /\ UNCHANGED x
Next == Inc \/ Flip
Spec == Init /\ [][Next]_vars /\ WF_vars(Next)
Often == []<>(ENABLED Flip /\ b)
====
SPECIFICATION Spec
PROPERTY Often
