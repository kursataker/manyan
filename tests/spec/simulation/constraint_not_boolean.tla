---- MODULE constraint_not_boolean ----
\* An action constraint that is not a boolean: TLC raises the error itself,
\* outside its evaluator, so its simulator reports it plainly.
\* options: -simulate num=2 -depth 10 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1
AC == IF x' < 2 THEN TRUE ELSE 7
====
INIT Init
NEXT Next
ACTION_CONSTRAINT AC
