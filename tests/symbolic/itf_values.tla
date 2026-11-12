\* options: -length 3
---- MODULE itf_values ----
\* Each kind of value as ITF writes it, by its type: a sequence as a
\* list, a tuple as #tup, a function as #map, a variant by its tag.
EXTENDS Integers, Sequences, Variants
CONSTANT
    \* @type: Int;
    N
VARIABLES
    \* @type: Int;
    x,
    \* @type: Seq(Int);
    s,
    \* @type: <<Int, Str>>;
    t,
    \* @type: Set(Int);
    set,
    \* @type: Int -> Bool;
    f,
    \* @type: { b: Int, a: Str };
    r,
    \* @type: A(UNIT) | B(Int);
    v,
    \* @type: M;
    m
Init == /\ x = -1 /\ s = <<3, 1>> /\ t = <<2, "z\"q">> /\ set = {3, 1, 2} /\ f = [i \in {2, 1} |-> i = 1]
        /\ r = [b |-> 1, a |-> "q"] /\ v = Variant("B", 7) /\ m = "m1_OF_M"
Next == /\ x' = x + N /\ s' = Append(s, x) /\ v' = Variant("A", UNIT) /\ UNCHANGED <<t, set, f, r, m>>
Inv == x < 1
====
CONSTANT N = 2
INIT Init
NEXT Next
INVARIANT Inv
