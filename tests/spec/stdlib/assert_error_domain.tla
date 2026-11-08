---- MODULE assert_error_domain ----
\* TLC's message for a function applied outside its domain ends with a
\* newline, which AssertError compares.
EXTENDS Integers, TLCExt
ASSUME LET f == [n \in -2..0 \cup 0..2 |-> n]
       IN AssertError("In applying the function\n(-2 :> -2 @@ -1 :> -1 @@ 0 :> 0 @@ 1 :> 1 @@ 2 :> 2),\nthe first argument is:\n-3\nwhich is not in its domain.\n", f[-3])
ASSUME LET f == [n \in -2..0 \cup 0..2 |-> n]
       IN ~AssertError("In applying the function\n(-2 :> -2 @@ -1 :> -1 @@ 0 :> 0 @@ 1 :> 1 @@ 2 :> 2),\nthe first argument is:\n-3\nwhich is not in its domain.", f[-3])
====
