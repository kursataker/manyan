---- MODULE rl_reward ----
\* Only TLC's reinforcement learning simulation, which a Java property
\* turns on, evaluates _RL_REWARD.
\* options: -simulate num=2 -depth 3 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Reward == x \div 0
====
INIT Init
NEXT Next
_RL_REWARD Reward
CHECK_DEADLOCK FALSE
