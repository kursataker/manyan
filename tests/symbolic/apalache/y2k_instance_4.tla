\* options: -length 10
\* apalache-mc check --out-dir=./test-out-dir --length=10 --output-traces --inv=Safety y2k_instance.tla: OK
---------------------------- MODULE y2k_instance_4 ----------------------------
(*
 * Another way to instantiate constants for apalache is to
 * use INSTANCE.
 *)
 
VARIABLE
    \* @type: Int;
    year,
    \* @type: Bool;
    hasLicense

INSTANCE y2k WITH BIRTH_YEAR <- 80, LICENSE_AGE <- 18

=============================================================================
\* Modification History
\* Last modified Tue Jan 07 11:24:55 CET 2020 by igor
\* Created Tue Jan 07 11:16:18 CET 2020 by igor
INIT Init
NEXT Next
INVARIANT Safety
