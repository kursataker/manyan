--------------- MODULE test33  -------------

(* Test of large sets of functions *)

EXTENDS TLC, Naturals

VARIABLE x

Init == /\ Print(<<"Computing", 7^6, "States">>, TRUE)
        /\ x \in [1..6 -> 1..7]   

Next == UNCHANGED x

Inv ==  TRUE
=========================================
  INIT
    Init
  
  NEXT
    Next
  
  INVARIANT Inv

 