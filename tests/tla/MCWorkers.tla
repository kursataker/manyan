-------------------------------- MODULE MCWorkers -------------------------------
(* Levels of three states, in batches of two, expanded by one worker and the
   search: each state with no successor, one, or two in either order, of
   two fingerprints; each fingerprint known before the level or not, its
   invariant failing or not; the second state's expansion failing or not. *)
EXTENDS Workers
Lists == {<<>>, <<1>>, <<1, 2>>, <<2, 1>>}
AllCases == [succ : [1..3 -> Lists], known : {{}, {2}}, bad : SUBSET {1, 2}, error : {{}, {2}}]
================================================================================
CONSTANT Cases <- AllCases
CONSTANT Workers = {w}
CONSTANT B = 2
SPECIFICATION Spec
INVARIANT Same OneClaim
PROPERTY Ends
CHECK_DEADLOCK FALSE
