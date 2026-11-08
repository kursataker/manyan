---- MODULE property_instance_operator_A ----
VARIABLE s
Property(ns(_), a) == [](ns(a))
Double(ns(_), a) == Property(ns, a)
====
