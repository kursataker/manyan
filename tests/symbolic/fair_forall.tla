\* options: -length 6
---- MODULE fair_forall ----
\* Each process, weakly fair, gets to done, though Next may stutter.
EXTENDS Naturals
CONSTANT
    \* @type: Set(Int);
    P
VARIABLE
    \* @type: Int -> Bool;
    done
Init == done = [p \in P |-> FALSE]
Finish(p) == ~done[p] /\ done' = [done EXCEPT ![p] = TRUE]
Next == (\E p \in P : Finish(p)) \/ UNCHANGED done
Spec == Init /\ [][Next]_done /\ \A p \in P : WF_done(Finish(p))
AllDone == <>(\A p \in P : done[p])
====
CONSTANT P = {1, 2}
SPECIFICATION Spec
PROPERTY AllDone
