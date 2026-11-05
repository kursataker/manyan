---- MODULE prefix_argument ----
\* Built-in prefix operators passed to an operator parameter.
EXTENDS TLC
Apply(F(_), a) == F(a)
ASSUME PrintT(<<Apply(SUBSET, {1}), Apply(UNION, {{1}, {2}}), Apply(DOMAIN, <<5, 6>>), Apply(~, TRUE)>>)
====
