\* options: -length 4
---- MODULE itf_lasso ----
\* A lasso in ITF: the states to the loop's last, and the loop's first.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x = 0
Next == x' = (x + 1) % 2
Never == <>(x = 5)
====
INIT Init
NEXT Next
PROPERTY Never
