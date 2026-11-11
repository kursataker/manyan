---- MODULE stack_parse ----
\* Expressions nested deeper than -stack: TLC's StackOverflowError, as
\* when its parser overflows its Java stack.
\* options: -stack 50
VARIABLE x
Init == x = ((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((1))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
Next == UNCHANGED x
====
INIT Init
NEXT Next
