---- MODULE strings_ops ----
EXTENDS Sequences, TLC
ASSUME PrintT(<<"ab" \o "cd", Len("hello"), "abc" = "abc", {"b", "a"} = {"a", "b"}>>)
====
