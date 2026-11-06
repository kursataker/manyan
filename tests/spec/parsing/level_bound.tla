---- MODULE level_bound ----
\* The range of a bounded variable may not be temporal; SANY names the
\* variable.
VARIABLE x
Always == [](x = 0)
Init == \A ok \in {Always} : ok
====
INIT Init
NEXT Init
