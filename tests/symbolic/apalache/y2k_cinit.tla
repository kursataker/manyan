\* options: -length 20 -cinit ConstInit
\* apalache-mc check --length=20 --inv=Safety --cinit=ConstInit y2k_cinit.tla: ERROR (12)
---------------------------- MODULE y2k_cinit ----------------------------
(*
 * Another way to instantiate constants for apalache is give it constraints
 * on the constants.
 *)
 
EXTENDS y2k

ConstInit ==
    /\ BIRTH_YEAR \in 0..99
    /\ LICENSE_AGE \in 10..99

=============================================================================
\* Modification History
\* Last modified Tue Jan 07 11:24:55 CET 2020 by igor
\* Created Tue Jan 07 11:16:18 CET 2020 by igor
INIT Init
NEXT Next
INVARIANT Safety
