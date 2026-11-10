# Full benchmark

`make full-bench` checks the models of
[tlaplus/Examples](https://github.com/tlaplus/Examples) with tpp and with
TLC, and writes this report. `make bench` ([benchmark](benchmark.md)) is the
quick one for day-to-day work; this one takes hours and asks a stronger
question: does tpp print what TLC prints on specs it was not written for?

Examples, CommunityModules and TLAPS are pinned in the Makefile. Each model
runs once with one worker, killed past 15 minutes or 4 GB. TLC's results are
cached under `.obj` and the diffs of the models that differ are written to
`.obj/fullbench.diff`. `ONLY=DieHard` runs the models whose path contains
DieHard; `MAX_RUNTIME=00:00:30` keeps those recorded to run in 30 seconds.

Of Examples' 243 models, it checks the 174 that TLC searches exhaustively
in a recorded 10 minutes or less, and the 6 that simulate or generate
behaviors, fixed by `-seed 1` and `num=`; the rest are Apalache's
(symbolic, 46), or longer or not timed (17). Where TLC is killed, there is
nothing to compare. One differs for a feature tpp lacks: the ALIAS of
ewd687a/EWD687a_anim reads `TLCGet("action")`, which tpp does not give in
a simulation. Three differ for reasons tpp keeps:

- SpanningTree/SpanTreeRandom draws its graph with `RandomElement` in a
  constant, which TLC draws anew each run, so its output is TLC's of one run.
- FiniteMonotonic/MCDistributedReplicatedLog: the same property is violated,
  but under its VIEW the two print different traces
  ([tlaplus#1045](https://github.com/tlaplus/tlaplus/issues/1045)).
- tower_of_hanoi/Hanoi.toolbox/Model_1/MC: TLC loads the model's own
  `Bits.class` and says so; tpp computes `Bits!And` from its definition,
  with the same result.

## Gaps

What the full bench does not yet compare, and what would close each gap.
A model leaves this list when it gets a row in the results.

| gap | models | to close |
|---|---:|---|
| runtime not recorded | 10 | Run each once under the 15-minute cap, and keep the ones that finish |
| TLC over 15 minutes | 4 | A longer cap for TLC, run once; its results stay cached |
| recorded over 10 minutes | 7 | A run with a larger `MAX_RUNTIME`, hours long |
| more than one worker | 174 | Run the checked models again with `-workers 4` |
| symbolic | 46 | None: these are Apalache's models, not TLC's |

- Runtime not recorded: CarTalkPuzzle Model_3, SpanTreeTest5Nodes,
  TestMCReachability, BPConProof, c1cs, TLCSailfish2, MC_HDiskSynod,
  EWD998ChanID_export, EWD998ChanID_shiviz, SmokeEWD998_SC.
- TLC over 15 minutes: ElevatorSafetyLarge (tpp 774 s), aba_asyn_byz
  (tpp 31 s), PConProof (tpp 185 s), MultiPaxos_MC (tpp over 15 minutes as
  well).
- Recorded over 10 minutes: MCKVSSafetyLarge (7 h 16 min),
  EnvironmentController (2 h), SlushLarge and EWD998 (50 min each),
  bcastFolklore (30 min), MCNanoLarge (20 min), ElevatorLivenessLarge
  (11 min).

<!-- results -->

tlaplus/Examples at ca168055339d, the exhaustive-search models recorded to run in at most 00:10:00, and the simulation and generation models with `-seed 1` and `num=` their traceCount, or 100. Measured on arm64 Darwin 25.5.0, one run, wall-clock seconds, one worker.

180 models: 172 same, 4 differs, 4 TLC. Where tpp prints what TLC prints, it is 20.1x as fast (geometric mean).

| model | distinct states | TLC | tpp | speedup | result |
|---|---:|---:|---:|---:|---|
| Bakery-Boulangerie/MCBakery | 655200 | 7.38 | 1.87 | 3.9x | same |
| Bakery-Boulangerie/MCBoulanger | 7866982 | 177.67 | 52.65 | 3.4x | same |
| CarTalkPuzzle/CarTalkPuzzle.toolbox/Model_1/MC | 0 | 0.50 | 0.01 | 87.9x | same |
| CarTalkPuzzle/CarTalkPuzzle.toolbox/Model_2/MC | 0 | 0.65 | 0.05 | 13.6x | same |
| Chameneos/Chameneos | 34534 | 0.87 | 0.06 | 14.9x | same |
| CheckpointCoordination/MCCheckpointCoordination | 901692 | 224.04 | 36.85 | 6.1x | same |
| CheckpointCoordination/MCCheckpointCoordinationFailure | 88152 | 12.66 | 2.30 | 5.5x | same |
| CigaretteSmokers/CigaretteSmokers | 6 | 0.33 | 0.01 | 57.9x | same |
| CoffeeCan/CoffeeCan1000Beans | 501500 | 31.81 | 0.93 | 34.2x | same |
| CoffeeCan/CoffeeCan100Beans | 5150 | 0.78 | 0.02 | 35.2x | same |
| CoffeeCan/CoffeeCan3000Beans | 4504500 | 778.00 | 15.30 | 50.9x | same |
| DieHard/DieHard | 14 | 0.35 | 0.01 | 60.3x | same |
| DieHard/MCDieHarder | 14 | 0.35 | 0.01 | 57.8x | same |
| DieHard/MCDieHardest | 11566 | 0.67 | 0.05 | 14.2x | same |
| DiningPhilosophers/DiningPhilosophers | 58 | 0.53 | 0.01 | 92.9x | same |
| DiningPhilosophers/DiningPhilosophersPCal | 67 | 0.39 | 0.01 | 67.2x | same |
| DiningPhilosophers/DiningPhilosophers_anim | 58 | 0.41 | 0.01 | 34.9x | same |
| DiningPhilosophers/DiningPhilosophers_stats |  | 0.57 | 0.02 | 24.8x | same |
| Disruptor/MCDisruptor_MPMC | 112929 | 2.64 | 0.54 | 4.9x | same |
| Disruptor/MCDisruptor_MPMC_liveliness | 14365 | 1.67 | 0.28 | 6.0x | same |
| Disruptor/MCDisruptor_SPMC | 8496 | 0.96 | 0.16 | 6.1x | same |
| FiniteMonotonic/MCCRDT | 25000 | 9.20 | 1.41 | 6.5x | same |
| FiniteMonotonic/MCDistributedReplicatedLog | 37 | 0.45 | 0.01 |  | differs |
| FiniteMonotonic/MCReplicatedLog | 1363 | 0.52 | 0.38 | 1.4x | same |
| FlashProtocol/MCFlashWithMutex | 2024 | 0.58 | 0.05 | 12.1x | same |
| FlashProtocol/MCFlashWithMutexLive | 8096 | 9.13 | 0.16 | 58.3x | same |
| GameOfLife/GameOfLife | 65536 | 8.42 | 2.52 | 3.3x | same |
| GermanProtocol/MCGermanCMPWithMutex | 1107 | 0.40 | 0.01 | 34.4x | same |
| GermanProtocol/MCGermanControl | 735 | 0.40 | 0.01 | 36.2x | same |
| GermanProtocol/MCGermanData | 852 | 0.47 | 0.01 | 40.3x | same |
| Huang/Huang | 81256 | 34.80 | 37.33 | 0.9x | same |
| KeyValueStore/MCKVSSafetyMedium | 17220672 | 826.26 | 210.71 | 3.9x | same |
| KeyValueStore/MCKVSSafetySmall | 3409605 | 130.22 | 26.68 | 4.9x | same |
| KeyValueStore/MCKVsnap | 32293 | 8.38 | 1.52 | 5.5x | same |
| KnuthYao/SimKnuthYao |  | 0.42 | 0.16 | 2.6x | same |
| LearnProofs/MCFindHighest | 742 | 0.50 | 0.02 | 21.0x | same |
| LeastCircularSubstring/MCLeastCircularSubstringMedium | 1007232 | 186.95 | 1.50 | 124.4x | same |
| LeastCircularSubstring/MCLeastCircularSubstringSmall | 8554 | 0.63 | 0.02 | 26.7x | same |
| LoopInvariance/MCBinarySearch | 27953 | 1.96 | 0.38 | 5.2x | same |
| LoopInvariance/MCQuicksort | 4548 | 4.75 | 0.49 | 9.7x | same |
| Majority/MCMajority | 2733 | 0.49 | 0.02 | 20.8x | same |
| MisraReachability/MCParReach | 393 | 9.82 | 0.01 | 842.5x | same |
| MisraReachability/MCReachabilityTestAllGraphs | 0 | 2.41 | 0.81 | 3.0x | same |
| MisraReachability/MCReachabilityTestRandomGraphs | 0 | 1.00 | 0.27 | 3.7x | same |
| MisraReachability/MCReachable | 8 | 58.07 | 0.01 | 5193.5x | same |
| MissionariesAndCannibals/MissionariesAndCannibals | 61 | 0.35 | 0.01 | 64.1x | same |
| Moving_Cat_Puzzle/CatEvenBoxes | 48 | 0.31 | 0.01 | 54.7x | same |
| Moving_Cat_Puzzle/CatOddBoxes | 30 | 0.31 | 0.01 | 55.3x | same |
| MultiCarElevator/ElevatorLivenessMedium | 4122 | 9.15 | 0.04 | 208.3x | same |
| MultiCarElevator/ElevatorSafetyLarge |  | 900.04 | 774.33 |  | TLC over 900 s |
| MultiCarElevator/ElevatorSafetyMedium | 17997111 | 504.57 | 150.90 | 3.3x | same |
| MultiCarElevator/ElevatorSafetySmall | 4122 | 0.72 | 0.04 | 17.8x | same |
| MultiPaxos-SMR/MultiPaxos_MC |  | 900.10 |  |  | TLC over 900 s |
| MultiPaxos-SMR/MultiPaxos_MC_small | 343796 | 21.62 | 4.83 | 4.5x | same |
| N-Queens/Queens.toolbox/FourQueens/MC | 40 | 0.34 | 0.01 | 58.9x | same |
| N-Queens/QueensPluscal.toolbox/FourQueens/MC | 40 | 0.38 | 0.01 | 68.6x | same |
| NanoBlockchain/MCNanoMedium | 530587 | 14.91 | 4.06 | 3.7x | same |
| NanoBlockchain/MCNanoSmall | 3003 | 0.57 | 0.05 | 12.0x | same |
| Paxos/MCConsensus | 4 | 0.35 | 0.01 | 62.6x | same |
| Paxos/MCPaxos | 25 | 0.45 | 0.01 | 38.6x | same |
| Paxos/MCVoting | 77 | 0.54 | 0.01 | 50.2x | same |
| PaxosHowToWinATuringAward/MCConsensus | 4 | 0.36 | 0.01 | 63.7x | same |
| PaxosHowToWinATuringAward/MCPaxosSmall | 185369 | 101.49 | 16.20 | 6.3x | same |
| PaxosHowToWinATuringAward/MCPaxosTiny | 3921 | 1.84 | 0.21 | 8.7x | same |
| PaxosHowToWinATuringAward/MCVoting | 6752 | 1.94 | 0.19 | 10.4x | same |
| Prisoners/Prisoners | 214 | 0.39 | 0.01 | 70.7x | same |
| Prisoners_Single_Switch/Prisoner | 16 | 0.34 | 0.01 | 61.7x | same |
| Prisoners_Single_Switch/PrisonerLightUnknown | 62 | 0.35 | 0.01 | 61.8x | same |
| Prisoners_Single_Switch/PrisonerSolo | 2 | 0.33 | 0.01 | 59.4x | same |
| Prisoners_Single_Switch/PrisonerSoloLightUnknown | 4 | 0.33 | 0.01 | 59.7x | same |
| ReadersWriters/MC | 21527 | 1.81 | 0.31 | 5.9x | same |
| SDP_Verification/SDP_Attack_New_Solution_Spec/MC | 2881 | 1.38 | 0.16 | 8.7x | same |
| SDP_Verification/SDP_Attack_Spec/MC | 526 | 0.74 | 0.02 | 33.9x | same |
| SimplifiedFastPaxos/FastPaxos | 25617 | 30.27 | 2.77 | 10.9x | same |
| SimplifiedFastPaxos/Paxos | 1207 | 1.24 | 0.11 | 11.6x | same |
| SingleLaneBridge/MC | 3605 | 3.19 | 0.05 | 60.4x | same |
| SlidingPuzzles/SlidingPuzzles | 24005 | 1.34 | 0.30 | 4.5x | same |
| SlidingPuzzles/SlidingPuzzles_anim | 24005 | 1.65 | 0.32 | 5.1x | same |
| SlushProtocol/SlushMedium | 9886336 | 182.47 | 38.61 | 4.7x | same |
| SlushProtocol/SlushSmall | 274678 | 6.64 | 1.04 | 6.4x | same |
| SpanningTree/SpanTree | 1236 | 0.86 | 0.01 | 76.4x | same |
| SpanningTree/SpanTreeRandom | 38376 | 9.40 | 0.55 |  | differs |
| SpanningTree/SpanTreeTest4Nodes | 3984 | 0.97 | 0.05 | 20.2x | same |
| SpecificationRefinement/abstract | 4 | 0.39 | 0.01 | 70.0x | same |
| SpecificationRefinement/bad | 12 | 0.40 | 0.01 | 71.6x | same |
| SpecificationRefinement/good | 9 | 0.41 | 0.01 | 73.2x | same |
| SpecifyingSystems/AdvancedExamples/MCInnerSequential | 3528 | 1.30 | 0.05 | 27.3x | same |
| SpecifyingSystems/AdvancedExamples/MCInnerSerial | 195 | 354.09 | 223.28 | 1.6x | same |
| SpecifyingSystems/AsynchronousInterface/AsynchInterface | 12 | 0.45 | 0.01 | 79.9x | same |
| SpecifyingSystems/AsynchronousInterface/Channel | 12 | 0.30 | 0.01 | 53.6x | same |
| SpecifyingSystems/AsynchronousInterface/PrintValues | 0 | 0.30 | 0.00 | 61.6x | same |
| SpecifyingSystems/CachingMemory/MCInternalMemory | 4408 | 0.40 | 0.01 | 34.8x | same |
| SpecifyingSystems/CachingMemory/MCWriteThroughCache | 5196 | 0.68 | 0.06 | 12.2x | same |
| SpecifyingSystems/FIFO/MCInnerFIFO | 3864 | 0.39 | 0.01 | 33.4x | same |
| SpecifyingSystems/HourClock/HourClock | 12 | 0.29 | 0.01 | 53.6x | same |
| SpecifyingSystems/HourClock/HourClock2 | 12 | 0.30 | 0.01 | 52.9x | same |
| SpecifyingSystems/Liveness/LiveHourClock | 12 | 0.31 | 0.01 | 56.3x | same |
| SpecifyingSystems/Liveness/MCLiveInternalMemory | 4408 | 0.99 | 0.08 | 12.6x | same |
| SpecifyingSystems/Liveness/MCLiveWriteThroughCache | 5196 | 1.36 | 0.27 | 5.1x | same |
| SpecifyingSystems/RealTime/MCRealTimeHourClock | 216 | 0.36 | 0.01 | 63.9x | same |
| SpecifyingSystems/SimpleMath/SimpleMath | 0 | 0.31 | 0.01 | 57.4x | same |
| SpecifyingSystems/TLC/ABCorrectness | 20 | 0.32 | 0.01 | 58.2x | same |
| SpecifyingSystems/TLC/MCAlternatingBit | 240 | 0.49 | 0.01 | 88.0x | same |
| StaleViewShrink/StaleShrink | 16848 | 0.70 | 0.06 | 12.2x | same |
| StaleViewShrink/StaleShrinkV6Wait2x | 43348 | 0.97 | 0.10 | 9.5x | same |
| StaleViewShrink/StaleShrinkV3Cascade | 32551 | 0.75 | 0.07 | 11.6x | same |
| StaleViewShrink/StaleShrinkV6TooShort | 2132 | 0.46 | 0.01 | 39.8x | same |
| Stones/Stones | 0 | 0.40 | 0.01 | 35.0x | same |
| TLC/TestGraphs | 2590 | 1.06 | 0.07 | 16.0x | same |
| TeachingConcurrency/Simple | 723 | 0.39 | 0.01 | 68.5x | same |
| TeachingConcurrency/SimpleRegular | 277726 | 3.74 | 0.77 | 4.8x | same |
| TransitiveClosure/TransitiveClosure | 0 | 0.63 | 0.10 | 6.3x | same |
| TwoPhase/MCTwoPhase | 4 | 0.31 | 0.01 | 55.5x | same |
| YoYo/MCYoYoNoPruning | 60 | 0.45 | 0.01 | 38.8x | same |
| YoYo/MCYoYoPruning | 102 | 0.52 | 0.01 | 44.9x | same |
| YoYo/YoYoAllGraphs | 26731 | 3.99 | 0.54 | 7.3x | same |
| aba-asyn-byz/aba_asyn_byz |  | 900.07 | 30.61 |  | TLC over 900 s |
| acp/ACP_NB_TLC | 4284 | 1.07 | 0.05 | 22.3x | same |
| acp/ACP_NB_WRONG_TLC | 11852 | 0.61 | 0.05 | 12.8x | same |
| acp/ACP_SB_TLC | 54944 | 0.95 | 0.10 | 9.2x | same |
| allocator/AllocatorImplementation | 17701 | 5.59 | 1.33 | 4.2x | same |
| allocator/AllocatorRefinement | 1690 | 0.90 | 0.13 | 6.8x | same |
| allocator/SchedulingAllocator | 1690 | 1.66 | 0.05 | 34.8x | same |
| allocator/SimpleAllocator | 400 | 0.86 | 0.02 | 36.6x | same |
| barriers/Barrier | 64 | 0.30 | 0.01 | 54.9x | same |
| barriers/Barriers | 29279 | 2.49 | 0.44 | 5.7x | same |
| bcastByz/bcastByz | 14424 | 9.76 | 0.34 | 28.8x | same |
| bcastByz/bcastByzNoBcast | 45 | 0.57 | 0.01 | 48.8x | same |
| bosco/bosco | 1072452 | 588.31 | 162.67 | 3.6x | same |
| braf/BufferedRandomAccessFile | 3316 | 36.47 | 8.92 | 4.1x | same |
| btree/MCbtree | 374727 | 28.51 | 4.49 | 6.3x | same |
| btree/kvstore | 2641 | 0.42 | 0.01 | 36.6x | same |
| byihive/VoucherCancel | 4199 | 0.56 | 0.06 | 10.0x | same |
| byihive/VoucherIssue | 4199 | 0.63 | 0.08 | 7.9x | same |
| byihive/VoucherLifeCycle | 64 | 0.30 | 0.01 | 53.9x | same |
| byihive/VoucherRedeem | 4199 | 0.54 | 0.06 | 9.5x | same |
| byihive/VoucherTransfer | 4197 | 0.57 | 0.06 | 10.2x | same |
| byzpaxos/Consensus | 4 | 0.37 | 0.01 | 66.8x | same |
| byzpaxos/PConProof |  | 900.07 | 185.17 |  | TLC over 900 s |
| byzpaxos/VoteProof | 6962 | 10.83 | 1.07 | 10.2x | same |
| cf1s-folklore/cf1s_folklore | 2057174 | 204.05 | 7.36 | 27.7x | same |
| chang_roberts/MCChangRoberts | 137 | 0.49 | 0.01 | 85.6x | same |
| dag-consensus/BlockDagTest | 0 | 0.40 | 0.01 | 70.9x | same |
| dag-consensus/TLCSailfish1 | 109604 | 39.34 | 10.00 | 3.9x | same |
| dijkstra-mutex/DijkstraMutex.toolbox/LSpec-model/MC | 90882 | 21.18 | 0.49 | 43.4x | same |
| dijkstra-mutex/DijkstraMutex.toolbox/Safety-4-processors/MC | 33288512 | 289.67 | 99.40 | 2.9x | same |
| echo/MCEcho | 75 | 0.70 | 0.02 | 29.0x | same |
| ewd426/TokenRing | 46656 | 1.56 | 0.13 | 11.5x | same |
| ewd426/SimTokenRing |  | 2.90 | 1.26 | 2.3x | same |
| ewd687a/MCEWD687a | 18028 | 3.49 | 0.28 | 12.6x | same |
| ewd687a/EWD687a_anim |  | 1.57 | 0.01 |  | differs |
| ewd840/EWD840 | 302 | 0.50 | 0.01 | 42.3x | same |
| ewd840/EWD840_json | 1566 | 0.43 | 0.01 | 36.8x | same |
| ewd840/SyncTerminationDetection | 129 | 0.36 | 0.01 | 65.4x | same |
| ewd840/EWD840_anim |  | 0.87 | 0.05 | 16.3x | same |
| ewd998/AsyncTerminationDetection | 4097 | 0.58 | 0.02 | 24.6x | same |
| ewd998/EWD998Small | 1520618 | 23.35 | 10.72 | 2.2x | same |
| ewd998/EWD998Chan | 1524022 | 775.49 | 145.29 | 5.3x | same |
| ewd998/EWD998ChanID | 14 | 0.68 | 0.02 | 28.7x | same |
| ewd998/EWD998ChanTrace |  | 0.41 | 0.01 | 35.0x | same |
| ewd998/EWD998PCal | 321370 | 26.81 | 7.20 | 3.7x | same |
| ewd998/SmokeEWD998 |  | 0.91 | 0.18 | 5.0x | same |
| glowingRaccoon/clean | 63 | 0.29 | 0.01 | 51.9x | same |
| glowingRaccoon/product | 305 | 0.42 | 0.02 | 18.0x | same |
| glowingRaccoon/stages | 83 | 0.33 | 0.01 | 64.6x | same |
| lamport_mutex/MCLamportMutex | 724274 | 10.60 | 3.19 | 3.3x | same |
| locks_auxiliary_vars/Lock | 12 | 0.33 | 0.01 | 58.5x | same |
| locks_auxiliary_vars/LockHS | 28 | 0.46 | 0.01 | 40.0x | same |
| locks_auxiliary_vars/Peterson | 42 | 0.39 | 0.01 | 69.5x | same |
| nbacc_ray97/nbacc_ray97 | 3016 | 0.51 | 0.02 | 22.4x | same |
| nbacg_guer01/nbacg_guer01 | 24922 | 2.34 | 0.08 | 29.7x | same |
| spanning/MC_spanning | 5 | 0.31 | 0.01 | 54.5x | same |
| sums_even/MC_sums_even | 0 | 0.51 | 0.05 | 10.8x | same |
| tcp/MCtcp | 1182 | 0.65 | 0.02 | 27.1x | same |
| tower_of_hanoi/Hanoi.toolbox/Model_1/MC | 233 | 0.38 | 0.02 |  | differs |
| tower_of_hanoi/Hanoi_anim | 25 | 0.47 | 0.01 | 40.2x | same |
| transaction_commit/2PCwithBTM | 1245 | 0.39 | 0.01 | 33.3x | same |
| transaction_commit/PaxosCommit | 1321761 | 121.98 | 20.00 | 6.1x | same |
| transaction_commit/TCommit | 34 | 0.31 | 0.01 | 54.2x | same |
| transaction_commit/TwoPhase | 288 | 0.33 | 0.01 | 58.8x | same |
