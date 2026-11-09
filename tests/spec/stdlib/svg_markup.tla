---- MODULE svg_markup ----
\* CommunityModules' SVGElemToString, which TLC implements in Java: an
\* element's markup, attribute names with - for _, a tuple's brackets in
\* the text escaped, then the children. Found by tlaplus/Examples'
\* SlidingPuzzles_anim.
EXTENDS SVG, TLC
Elem(name, attrs, children, text) == [name |-> name, attrs |-> attrs, children |-> children, innerText |-> text]
Dot == Elem("circle", [r |-> "2", cx |-> "1"], <<>>, "")
ASSUME PrintT(SVGElemToString(Elem("g", [stroke_width |-> "3", fill |-> "red"], <<Dot, Elem("text", <<>>, <<>>, "<<1, 2>>")>>, "")))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
