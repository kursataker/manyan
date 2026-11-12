---- MODULE rl_reward_undefined ----
\* A _RL_REWARD name must be defined.
VARIABLE x
Init == x = 0
====
INIT Init
NEXT Init
_RL_REWARD Nope
