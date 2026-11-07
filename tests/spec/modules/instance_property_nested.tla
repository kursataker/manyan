---- MODULE instance_property_nested ----
\* A property through two instances, I!Refinement with Refinement == M!Spec,
\* split as one: its [][Next]_vars is an action property. Found by
\* tlaplus/Examples' TestGraphs.
VARIABLE y, z
INSTANCE Mid
====
SPECIFICATION Spec
PROPERTY Refinement
