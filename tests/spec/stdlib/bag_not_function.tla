---- MODULE bag_not_function ----
\* BagIn of a value no function is the Java method's NullPointerException.
EXTENDS Naturals, Bags
ASSUME BagIn(1, {})
====
