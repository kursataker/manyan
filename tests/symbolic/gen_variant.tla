\* options: -length 2
---- MODULE gen_variant ----
\* Gen of a variant: a value of one of its tags.
EXTENDS Integers, Variants, Apalache
VARIABLE
    \* @type: A(Int) | B(Bool) | C(<<Int, Int>>);
    v
Init == v = Gen(1) /\ VariantTag(v) = "A"
Next == v' = Gen(1) /\ VariantTag(v') # VariantTag(v)
Inv == ~(VariantTag(v) = "C" /\ VariantGetUnsafe("C", v)[2] = 3)
====
INIT Init
NEXT Next
INVARIANT Inv
