---- MODULE json_to_json ----
\* Json's ToJson, ToJsonArray and ToJsonObject, as its Java writes them with Gson.
EXTENDS Json, TLC, Sequences, Integers
CONSTANT m
ASSUME PrintT(ToJson([a |-> 1, b |-> "x\"y", c |-> <<1, TRUE>>]))
ASSUME PrintT(ToJson(<<>>))
ASSUME PrintT(ToJson({}))
ASSUME PrintT(ToJson({3, 1, 2}))
ASSUME PrintT(ToJson({{}, {1}}))
ASSUME PrintT(ToJson({<<1>>, <<2>>}))
ASSUME PrintT(ToJson([n \in {"p", "q"} |-> {n}]))
ASSUME PrintT(ToJson([n \in {"b", "a", "c"} |-> 0]))
ASSUME PrintT(ToJson([n \in 1..3 |-> n * 2]))
ASSUME PrintT(ToJson([n \in {2, 3} |-> n]))
ASSUME PrintT(ToJson([n \in 0..1 |-> n]))
ASSUME PrintT(ToJson([n \in {} |-> n]))
ASSUME PrintT(ToJson([n \in {<<1, 2>>} |-> n]))
ASSUME PrintT(ToJson([n \in {m} |-> 1]))
ASSUME PrintT(ToJson(<<[x |-> -1], "s\n\t", 1..2>>))
ASSUME PrintT(ToJson(<<"\r", "a\\b", "é/">>))
ASSUME PrintT(ToJson(42))
ASSUME PrintT(ToJson("str"))
ASSUME PrintT(ToJson(m))
ASSUME PrintT(ToJson(SUBSET {1, 2}))
ASSUME PrintT(ToJson([{1, 2} -> {TRUE}]))
ASSUME PrintT(ToJsonArray(<<1, 2>>))
ASSUME PrintT(ToJsonArray({1, 2}))
ASSUME PrintT(ToJsonArray([n \in {"a"} |-> 1]))
ASSUME PrintT(ToJsonObject([a |-> 1]))
ASSUME PrintT(ToJsonObject(<<1, 2>>))
ASSUME PrintT(ToJsonObject([n \in 1..2 |-> n]))
ASSUME PrintT(ToJsonObject([n \in {3, 4} |-> n]))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
CONSTANT m = m
