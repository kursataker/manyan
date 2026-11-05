---- MODULE strings ----
EXTENDS TLC
ASSUME PrintT(<<"", "plain", "quote\"inside", "back\\slash", "tab\tend">>)
ASSUME PrintT("a" = "a" /\ "a" # "b")
====
