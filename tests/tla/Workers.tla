-------------------------------- MODULE Workers --------------------------------
(* One level of tpp's parallel search (tpp.c, "workers"; docs/manual.md,
   Workers). The level's states 1..L are cut into batches of B. Processes
   take batches in turn; the search is one of them, and takes one only
   while it waits for the batch it merges next. A process expands each
   state of its batch: a successor already in the fingerprint table is
   "seen"; another is "new", and the first process to reach it claims it
   (CAS) and then settles the claim, checking its invariant. The table is
   the search's, which it writes during the level, so a process may or may
   not see a state the search added meanwhile. A process may give a batch
   back, or give up on a state, for any reason: no room, a string it
   cannot intern, output, an error. The search merges the batches in
   order: a state whose expansion is clean, all its new successors' claims
   settled, it merges, waiting while a claim is still being settled; any
   other it expands again itself, as one process does. It stops at the
   first error and lets the processes finish their batches.

   The level must end as one process expanding the states in order ends
   it: the same new states, in the same order, from the same parents, and
   the same error. *)
EXTENDS Naturals, Sequences, FiniteSets

CONSTANTS Cases,    \* a set of [succ, known, bad, error]: one level each
          Workers,  \* the processes but the search
          B         \* the batch size

Search == "search"
Procs == Workers \cup {Search}

VARIABLES c,        \* the case
          next,     \* the next batch to take
          owner,    \* each batch's process, or "back" when given back
          done,     \* the batches expanded
          results,  \* each state's expansion: clean, and what it found
          claims,   \* each fingerprint's claim: none, claimed, settled, failed
          pc, bat, idx, k, found,   \* each process's place in its batch
          sb, si,   \* the batch and the state the search merges next
          acc,      \* what the search has done: the states added, its status, at which state
          phase     \* the search: merging, stopping, stopped
vars == <<c, next, owner, done, results, claims, pc, bat, idx, k, found, sb, si, acc, phase>>

L == Len(c.succ)
NB == (L + B - 1) \div B
First(b) == (b - 1) * B + 1
Last(b) == IF b * B < L THEN b * B ELSE L
Fps(a) == c.known \cup {a.added[j][1] : j \in DOMAIN a.added}

(* One process expanding state i after the states before it, from the
   successor k on: the reference, and what the search does itself. *)
RECURSIVE Expand(_, _, _)
Expand(a, i, j) ==
    IF a.status # "ok" \/ j > Len(c.succ[i]) THEN a
    ELSE LET fp == c.succ[i][j] IN
         IF fp \in Fps(a) THEN Expand(a, i, j + 1)
         ELSE IF fp \in c.bad THEN [a EXCEPT !.status = "invariant", !.at = i]
         ELSE Expand([a EXCEPT !.added = Append(@, <<fp, i>>)], i, j + 1)
Step(a, i) ==
    IF a.status # "ok" THEN a
    ELSE IF i \in c.error THEN [a EXCEPT !.status = "error", !.at = i]
    ELSE Expand(a, i, 1)
Start == [added |-> <<>>, status |-> "ok", at |-> 0]
RECURSIVE Sequential(_)
Sequential(i) == IF i = 0 THEN Start ELSE Step(Sequential(i - 1), i)

AllFps == UNION {{c.succ[i][j] : j \in DOMAIN c.succ[i]} : i \in DOMAIN c.succ}

Init ==
    /\ c \in Cases
    /\ next = 1
    /\ owner = [b \in 1..NB |-> "none"]
    /\ done = [b \in 1..NB |-> FALSE]
    /\ results = [i \in 1..L |-> [clean |-> FALSE, found |-> <<>>]]
    /\ claims = [fp \in AllFps |-> "none"]
    /\ pc = [p \in Procs |-> "idle"]
    /\ bat = [p \in Procs |-> 0]
    /\ idx = [p \in Procs |-> 0]
    /\ k = [p \in Procs |-> 0]
    /\ found = [p \in Procs |-> <<>>]
    /\ sb = 1 /\ si = 1
    /\ acc = Start
    /\ phase = "merging"

Waiting == phase = "merging" /\ sb <= NB /\ ~done[sb]

(* A process takes the next batch, or gives it back at once. *)
Take(p) ==
    /\ pc[p] = "idle" /\ next <= NB
    /\ p = Search => Waiting
    /\ next' = next + 1
    /\ \/ /\ owner' = [owner EXCEPT ![next] = "back"]
          /\ done' = [done EXCEPT ![next] = TRUE]
          /\ UNCHANGED <<pc, bat, idx>>
       \/ /\ owner' = [owner EXCEPT ![next] = p]
          /\ pc' = [pc EXCEPT ![p] = "state"]
          /\ bat' = [bat EXCEPT ![p] = next]
          /\ idx' = [idx EXCEPT ![p] = First(next)]
          /\ UNCHANGED done
    /\ UNCHANGED <<c, results, claims, k, found, sb, si, acc, phase>>

Finish(p, clean) ==
    /\ results' = [results EXCEPT ![idx[p]] = [clean |-> clean, found |-> IF clean THEN found[p] ELSE <<>>]]
    /\ idx' = [idx EXCEPT ![p] = @ + 1]
    /\ pc' = [pc EXCEPT ![p] = "state"]

(* The next state of the batch: done with the batch, or start the state,
   which gives up at once when its expansion fails. *)
NextState(p) ==
    /\ pc[p] = "state"
    /\ IF idx[p] > Last(bat[p])
       THEN /\ done' = [done EXCEPT ![bat[p]] = TRUE]
            /\ pc' = [pc EXCEPT ![p] = "idle"]
            /\ UNCHANGED <<results, idx, k, found>>
       ELSE IF idx[p] \in c.error
       THEN Finish(p, FALSE) /\ UNCHANGED <<done, k, found>>
       ELSE /\ pc' = [pc EXCEPT ![p] = "succ"]
            /\ k' = [k EXCEPT ![p] = 1]
            /\ found' = [found EXCEPT ![p] = <<>>]
            /\ UNCHANGED <<done, results, idx>>
    /\ UNCHANGED <<c, next, owner, claims, bat, sb, si, acc, phase>>

(* The next successor of the state, or the end of it. *)
Successor(p) ==
    /\ pc[p] = "succ"
    /\ LET i == idx[p]
           fp == c.succ[i][k[p]]
           Record(kind) == found' = [found EXCEPT ![p] = Append(@, [fp |-> fp, kind |-> kind])]
       IN IF k[p] > Len(c.succ[i])
          THEN Finish(p, TRUE) /\ UNCHANGED <<claims, k, found>>
          ELSE \/ Finish(p, FALSE) /\ UNCHANGED <<claims, k, found>>   \* gives up
               \/ /\ fp \in Fps(acc)   \* in the table; the search's additions may go unread
                  /\ Record("seen") /\ k' = [k EXCEPT ![p] = @ + 1]
                  /\ UNCHANGED <<claims, results, idx, pc>>
               \/ /\ fp \notin c.known
                  /\ IF claims[fp] = "none"
                     THEN /\ claims' = [claims EXCEPT ![fp] = "claimed"]
                          /\ pc' = [pc EXCEPT ![p] = "settle"]
                          /\ UNCHANGED k
                     ELSE k' = [k EXCEPT ![p] = @ + 1] /\ UNCHANGED <<claims, pc>>
                  /\ Record("new")
                  /\ UNCHANGED <<results, idx>>
    /\ UNCHANGED <<c, next, owner, done, bat, sb, si, acc, phase>>

(* The process that claimed the successor checks its invariant. *)
Settle(p) ==
    /\ pc[p] = "settle"
    /\ LET fp == c.succ[idx[p]][k[p]]
       IN claims' = [claims EXCEPT ![fp] = IF fp \in c.bad THEN "failed" ELSE "settled"]
    /\ k' = [k EXCEPT ![p] = @ + 1]
    /\ pc' = [pc EXCEPT ![p] = "succ"]
    /\ UNCHANGED <<c, next, owner, done, results, bat, idx, found, sb, si, acc, phase>>

(* The search merges state si of batch sb, or expands it itself. *)
Merge ==
    /\ phase = "merging" /\ acc.status = "ok" /\ sb <= NB /\ done[sb] /\ pc[Search] = "idle"
    /\ LET r == results[si]
           news == {r.found[j].fp : j \in {j \in DOMAIN r.found : r.found[j].kind = "new"}}
           usable == owner[sb] # "back" /\ r.clean /\ \A fp \in news : claims[fp] # "failed"
           Add(a, j) == IF j > Len(r.found) THEN a
                        ELSE LET fp == r.found[j].fp IN
                             IF fp \in Fps(a) THEN a ELSE [a EXCEPT !.added = Append(@, <<fp, si>>)]
           RECURSIVE Merged(_, _)
           Merged(a, j) == IF j > Len(r.found) THEN a ELSE Merged(Add(a, j), j + 1)
       IN /\ usable => \A fp \in news : claims[fp] # "claimed"   \* waits for its claims
          /\ acc' = IF usable THEN Merged(acc, 1) ELSE Step(acc, si)
    /\ IF si = Last(sb) THEN sb' = sb + 1 /\ si' = si + 1 ELSE si' = si + 1 /\ UNCHANGED sb
    /\ UNCHANGED <<c, next, owner, done, results, claims, pc, bat, idx, k, found, phase>>

(* After the level, or its first error, the processes take no more
   batches; the search waits for them to finish theirs. *)
Stop ==
    /\ phase = "merging" /\ (sb > NB \/ acc.status # "ok") /\ pc[Search] = "idle"
    /\ phase' = "stopping" /\ next' = NB + 1
    /\ UNCHANGED <<c, owner, done, results, claims, pc, bat, idx, k, found, sb, si, acc>>

Stopped ==
    /\ phase = "stopping" /\ \A p \in Procs : pc[p] = "idle"
    /\ phase' = "stopped"
    /\ UNCHANGED <<c, next, owner, done, results, claims, pc, bat, idx, k, found, sb, si, acc>>

Proc(p) == Take(p) \/ NextState(p) \/ Successor(p) \/ Settle(p)
Next == (\E p \in Procs : Proc(p)) \/ Merge \/ Stop \/ Stopped
Spec == Init /\ [][Next]_vars /\ \A p \in Procs : WF_vars(Proc(p)) /\ WF_vars(Merge \/ Stop \/ Stopped)

-------------------------------------------------------------------------------
Same == phase = "stopped" => acc = Sequential(L)
OneClaim == \A fp \in DOMAIN claims : claims[fp] # "none" => fp \notin c.known
Ends == <>(phase = "stopped")
================================================================================
