/* tpp-std: the operators TLC computes in Java for its modules and for
   CommunityModules', as native operators of tpp.h. See docs/std.md. */
#ifndef TPP_STD_H
#define TPP_STD_H

#include "tpp.h"

/* Defines them all with tpp_define. Returns 0, or -1 when out of memory. */
int tpp_std_init(void);

#endif
