------------------------------ MODULE InitSlices ------------------------------
(* How Manyan's workers compute the initial states (manyan.c, "Init sliced
   among the workers"; init_slices, worker_init, init_parallel). Init is a
   tree of choices (an element of x \in S, a disjunct, a binding of \E),
   each alternative the rest of the path; a path ends in a state or fails.
   One process takes the alternatives in order and finds the states at the
   leaves. Sliced to depth D, each alternative of a choice above depth D is
   numbered in the order one process takes them, and a state belongs to the
   slice of the last number taken on its path. A process computing the
   slices [lo, hi) takes every alternative above depth D - 1, and of those
   at depth D - 1 only the ones it owns; it keeps the states of its slices
   only. The counting pass takes every alternative above depth D, and
   stops at a choice at depth D, noting that there is one: its count is
   the number of slices.

   The slices, in order, must give the states one process finds, in its
   order: computed one at a time (the search redoing a slice) or a batch
   at a time (a worker). *)
EXTENDS Integers, Sequences

CONSTANTS Trees,   \* the Inits to check: a leaf [s |-> state], a failure [s |-> 0], or [alts |-> <<tree, ...>>]
          Depths   \* the depths to slice them to

VARIABLE t
vars == <<t>>

IsChoice(x) == "alts" \in DOMAIN x

(* The states one process finds. *)
RECURSIVE Serial(_), SerialAlts(_, _)
Serial(x) == IF IsChoice(x) THEN SerialAlts(x.alts, 1) ELSE IF x.s = 0 THEN <<>> ELSE <<x.s>>
SerialAlts(as, i) == IF i > Len(as) THEN <<>> ELSE Serial(as[i]) \o SerialAlts(as, i + 1)

(* The counting pass from x at depth d, a the slices numbered so far
   (choice_begin, choice_take with slice_counting). *)
RECURSIVE Count(_, _, _, _), CountAlts(_, _, _, _, _)
Count(x, d, a, D) == IF IsChoice(x) /\ d < D THEN CountAlts(x.alts, 1, d, a, D) ELSE a
CountAlts(as, i, d, a, D) ==
    IF i > Len(as) THEN a ELSE CountAlts(as, i + 1, d, Count(as[i], d + 1, a + 1, D), D)

(* A process computing the slices [lo, hi) from x at depth d, a its last
   number and its states (choice_take, worker_emit's slice_owned). *)
Owned(n, lo, hi) == n >= lo /\ n < hi
RECURSIVE Walk(_, _, _, _, _, _), WalkAlts(_, _, _, _, _, _, _)
Walk(x, d, a, D, lo, hi) ==
    IF IsChoice(x) THEN WalkAlts(x.alts, 1, d, a, D, lo, hi)
    ELSE IF x.s # 0 /\ Owned(a.n, lo, hi) THEN [a EXCEPT !.out = Append(@, x.s)]
    ELSE a
WalkAlts(as, i, d, a, D, lo, hi) ==
    IF i > Len(as) THEN a
    ELSE LET b == IF d < D THEN [a EXCEPT !.n = @ + 1] ELSE a
             take == d # D - 1 \/ Owned(b.n, lo, hi)
         IN WalkAlts(as, i + 1, d, IF take THEN Walk(as[i], d + 1, b, D, lo, hi) ELSE b, D, lo, hi)

Slices(x, D, lo, hi) == Walk(x, 0, [n |-> -1, out |-> <<>>], D, lo, hi).out

RECURSIVE InOrder(_, _, _, _, _)
InOrder(x, D, s, total, per) ==
    IF s >= total THEN <<>> ELSE Slices(x, D, s, s + per) \o InOrder(x, D, s + per, total, per)

Init == t \in Trees
Next == UNCHANGED vars
Spec == Init /\ [][Next]_vars

(* Sliced Init: the slices one at a time, or two to a batch, give the
   states in one process's order. *)
Same == \A D \in Depths :
            LET total == Count(t, 0, -1, D) + 1
            IN total = 0 \/ (InOrder(t, D, 0, total, 1) = Serial(t) /\ InOrder(t, D, 0, total, 2) = Serial(t))
================================================================================
