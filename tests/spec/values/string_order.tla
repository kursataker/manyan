---- MODULE string_order ----
EXTENDS TLC
VARIABLE zeta
Beta == "beta"
ASSUME PrintT({"gamma", "alpha", "zeta", "Beta", "beta", "delta"})
ASSUME PrintT([gamma |-> 1, alpha |-> 2, zeta |-> 3])
Init == zeta = 0
Next == UNCHANGED zeta
====
INIT Init
NEXT Next
