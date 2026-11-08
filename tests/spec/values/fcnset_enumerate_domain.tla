---- MODULE fcnset_enumerate_domain ----
\* Enumerating [D -> R], TLC names a domain it cannot hold as its
\* elements; another domain fails as TLC enumerates it.
EXTENDS Integers, Sequences, TLCExt
ASSUME AssertError("Attempted to enumerate S \\ T when S:\nNat\nis not enumerable.", CHOOSE f \in [Nat \ {0} -> {1}] : TRUE)
ASSUME AssertError("Attempted to enumerate { x \\in S : p(x) } when S:\nNat\nis not enumerable", CHOOSE f \in [{x \in Nat : x > 0} -> {1}] : TRUE)
ASSUME AssertError("Attempted to enumerate a set of the form [D -> R],but the domain D:\nInt\ncannot be enumerated.", CHOOSE f \in [Int -> {1}] : TRUE)
ASSUME AssertError("Attempted to enumerate a set of the form [D -> R],but the domain D:\nSeq({1})\ncannot be enumerated.", CHOOSE f \in [Seq({1}) -> {1}] : TRUE)
ASSUME AssertError("Attempted to enumerate a set of the form [D -> R],but the range R:\nNat\ncannot be enumerated.", CHOOSE f \in [{1} -> Nat] : TRUE)
====
