---- MODULE instance_subscript ----
\* A subscript may name an instance's definition: WF_I!vars(A), [A]_I!vars.
EXTENDS Naturals
VARIABLE x
---- MODULE Inner ----
VARIABLE y
vars == <<y>>
====
I == INSTANCE Inner WITH y <- x
Next == x < 3 /\ x' = x + 1
Spec == x = 0 /\ [][Next]_I!vars /\ WF_I!vars(Next)
Live == <>(x = 3)
Never == []<><<Next>>_I!vars
====
SPECIFICATION Spec
PROPERTY Live Never
CHECK_DEADLOCK FALSE
