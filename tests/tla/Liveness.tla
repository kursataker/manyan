------------------------------ MODULE Liveness ---------------------------------
(* How Manyan finds a fair cycle in the product of the state graph and a
   property's tableau (manyan.c, scc_search, accept_component and
   print_counterexample), on an explicit product graph. Its nodes reachable
   from the initial ones are split into strongly connected components; a
   component is accepted when it has an edge inside it, a node that fulfils
   the eventuality, and meets each fairness condition in order: a step of
   the action on an edge inside it, or for weak fairness a node where the
   action is disabled, for strong fairness no node where it is enabled.
   The first condition it fails rejects it if weak; if strong, the search
   goes on in the components of its nodes where the action is disabled.

   There is such a component exactly when the graph has a fair cycle that
   fulfils the eventuality, and the cycle Manyan prints in one is fair
   (live_walk): it takes, for each weak fairness condition, a step of the
   action or an edge from a node where it is disabled, and for each strong
   one a step if the component has one. *)
EXTENDS Integers, Sequences

CONSTANT Graphs   \* [succ, init, done, fairs]: succ[n] are n's successors, done the
                  \* nodes fulfilling the eventuality, fairs a sequence of
                  \* [strong, enabled, step]: step a set of edges <<m, n>>

VARIABLE g
vars == <<g>>

Edges(C) == {e \in C \X C : e[2] \in g.succ[e[1]]}

(* The nodes reachable from X, X included, by edges within S. *)
RECURSIVE Closure(_, _)
Closure(X, S) == LET Y == X \cup UNION {g.succ[n] \cap S : n \in X}
                 IN IF Y = X THEN X ELSE Closure(Y, S)
Reach(m, S) == Closure(g.succ[m] \cap S, S)   \* by one edge or more

Components(S) == {{n \in S : n = m \/ (n \in Reach(m, S) /\ m \in Reach(n, S))} : m \in S}
Reachable == Closure(g.init, DOMAIN g.succ)

Step(C, f) == Edges(C) \cap f.step # {}
Met(C, f) == Step(C, f) \/ IF f.strong THEN C \cap f.enabled = {} ELSE ~(C \subseteq f.enabled)

(* The components accept_component accepts within C. *)
RECURSIVE Accepted(_)
Accepted(C) ==
    IF Edges(C) = {} \/ C \cap g.done = {} THEN {}
    ELSE LET unmet == {i \in 1..Len(g.fairs) : ~Met(C, g.fairs[i])}
         IN IF unmet = {} THEN {C}
            ELSE LET f == g.fairs[CHOOSE i \in unmet : \A j \in unmet : i <= j]
                 IN IF ~f.strong THEN {}
                    ELSE UNION {Accepted(D) : D \in Components(C \ f.enabled)}

Found == UNION {Accepted(C) : C \in Components(Reachable)}

(* A cycle that goes through exactly the nodes C, and may take every edge
   between them, is fair and fulfils the eventuality. *)
FairCycle(C) == /\ C # {}
                /\ \A m, n \in C : n \in Reach(m, C)
                /\ C \cap g.done # {}
                /\ \A i \in 1..Len(g.fairs) : Met(C, g.fairs[i])

(* What live_walk's cycle takes in A for the condition f. *)
Route(A, f) == IF Step(A, f) \/ (~f.strong /\ ~(A \subseteq f.enabled)) THEN "edge" ELSE "nothing"

Init == g \in Graphs
Next == UNCHANGED vars
Spec == Init /\ [][Next]_vars

Complete == (Found # {}) = (\E C \in SUBSET Reachable : FairCycle(C))

(* Whatever other nodes of A the cycle goes through, it is fair. *)
LassoFair == \A A \in Found : \A i \in 1..Len(g.fairs) :
                 Route(A, g.fairs[i]) # "nothing" \/ A \cap g.fairs[i].enabled = {}
================================================================================
