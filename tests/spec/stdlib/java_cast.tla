---- MODULE java_cast ----
\* TLC casts each argument of a Java override to its parameter's class,
\* and reports Java's error with the method: JsonDeserialize takes a
\* StringValue.
EXTENDS Json, TLC
ASSUME PrintT(JsonDeserialize(1))
====
