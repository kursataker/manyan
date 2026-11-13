# Full benchmark

`make full-bench` checks the models of
[tlaplus/Examples](https://github.com/tlaplus/Examples) with Manyan and with
TLC, and writes this report. `make bench` ([benchmark](benchmark.md)) is the
quick one for day-to-day work; this one takes hours and asks a stronger
question: does Manyan print what TLC prints on specs it was not written for?

Examples, CommunityModules and TLAPS are pinned in the Makefile. Each model
runs once with one worker, killed past 15 minutes or 4 GB. TLC's results are
cached under `.obj` and the diffs of the models that differ are written to
`.obj/fullbench.diff`. `ONLY=DieHard` runs the models whose path contains
DieHard; `MAX_RUNTIME=00:00:30` keeps those recorded to run in 30 seconds.

Of Examples' 243 models, it checks the 174 that TLC searches exhaustively
in a recorded 10 minutes or less, and the 6 that simulate or generate
behaviors, fixed by `-seed 1` and `num=`; the rest are Apalache's
(symbolic, 46), or longer or not timed (17). Where TLC is killed, there is
nothing to compare. Four differ, for reasons Manyan keeps:

- ewd687a/EWD687a_anim: its ALIAS lays out its graph with SVG's
  `NodesOfDirectedMultiGraph`, which TLC computes with jungrapht's
  Sugiyama layout, a Java library's heuristics, and Manyan from its
  definition, a `CHOOSE` it cannot enumerate. Copying a layout library
  for one animation is not planned.
- SpanningTree/SpanTreeRandom draws its graph with `RandomElement` in a
  constant, which TLC draws anew each run, so its output is TLC's of one run.
- FiniteMonotonic/MCDistributedReplicatedLog: the same property is violated,
  but under its VIEW the two print different traces
  ([tlaplus#1045](https://github.com/tlaplus/tlaplus/issues/1045)).
- tower_of_hanoi/Hanoi.toolbox/Model_1/MC: TLC loads the model's own
  `Bits.class` and says so; Manyan computes `Bits!And` from its definition,
  with the same result.

## Gaps

What the full bench does not yet compare, and what would close each gap.
A model leaves this list when it gets a row in the results.

| gap | models | to close |
|---|---:|---|
| runtime not recorded | 10 | Run each once under the 15-minute cap, and keep the ones that finish |
| TLC over 15 minutes | 4 | A longer cap for TLC, run once; its results stay cached |
| recorded over 10 minutes | 7 | A run with a larger `MAX_RUNTIME`, hours long |
| more than one worker | 7 | Run those recorded over a minute with `-workers 4` too |
| symbolic | 46 | None: these are Apalache's models, not TLC's |

- More than one worker: the 167 models recorded to run in a minute or
  less were run once with `-workers 4` (by an engine wrapping
  `manyan -workers 4`): 160 print what TLC prints, as with one worker,
  and the four that differ with one worker differ alike; the run found
  TLCSailfish1's parse failure, since fixed. SlushMedium and
  MCBoulanger went past the 4 GB guard, which sums the resident memory
  of the worker processes, pages they share counted in each: on
  SlushSmall, 242 MB summed with four workers where the search process
  holds 67 MB, as it does alone (65 MB). With one worker both run in
  less than 2 GB.
- Runtime not recorded: CarTalkPuzzle Model_3, SpanTreeTest5Nodes,
  TestMCReachability, BPConProof, c1cs, TLCSailfish2, MC_HDiskSynod,
  EWD998ChanID_export, EWD998ChanID_shiviz, SmokeEWD998_SC. Run once by
  hand: SpanTreeTest5Nodes prints what TLC prints (TLC 590 s, Manyan
  168 s); CarTalkPuzzle Model_3 filters 121^5 functions, past 15 minutes
  for TLC; TestMCReachability runs past 15 minutes for TLC as well;
  SmokeEWD998_SC runs TLC itself through IOExec; the others run past 3
  minutes or 2 GB in Manyan.
- TLC over 15 minutes: ElevatorSafetyLarge (Manyan 774 s), aba_asyn_byz
  (Manyan 31 s), PConProof (Manyan 185 s), MultiPaxos_MC (Manyan over 15 minutes as
  well).
- Recorded over 10 minutes: MCKVSSafetyLarge (7 h 16 min),
  EnvironmentController (2 h), SlushLarge and EWD998 (50 min each),
  bcastFolklore (30 min), MCNanoLarge (20 min), ElevatorLivenessLarge
  (11 min).

<!-- results -->

tlaplus/Examples at ca168055339d, the exhaustive-search models recorded to run in at most 00:10:00, and the simulation and generation models with `-seed 1` and `num=` their traceCount, or 100. Measured on arm64 Darwin 25.5.0, one run, wall-clock seconds, one worker.

180 models: 172 same, 4 differs, 4 TLC. Where manyan prints what TLC prints, it is 20.4x as fast (geometric mean).

| model | distinct states | TLC | Manyan | speedup | result |
|---|---:|---:|---:|---:|---|
| Bakery-Boulangerie/MCBakery | 655200 | 6.94 | 1.88 | 3.7x | same |
| Bakery-Boulangerie/MCBoulanger | 7866982 | 159.75 | 60.47 | 2.6x | same |
| CarTalkPuzzle/CarTalkPuzzle.toolbox/Model_1/MC | 0 | 0.43 | 0.01 | 81.9x | same |
| CarTalkPuzzle/CarTalkPuzzle.toolbox/Model_2/MC | 0 | 0.60 | 0.05 | 12.6x | same |
| Chameneos/Chameneos | 34534 | 0.82 | 0.08 | 10.8x | same |
| CheckpointCoordination/MCCheckpointCoordination | 901692 | 182.32 | 36.28 | 5.0x | same |
| CheckpointCoordination/MCCheckpointCoordinationFailure | 88152 | 12.39 | 2.25 | 5.5x | same |
| CigaretteSmokers/CigaretteSmokers | 6 | 0.33 | 0.01 | 63.2x | same |
| CoffeeCan/CoffeeCan1000Beans | 501500 | 30.21 | 0.92 | 32.8x | same |
| CoffeeCan/CoffeeCan100Beans | 5150 | 0.80 | 0.01 | 54.7x | same |
| CoffeeCan/CoffeeCan3000Beans | 4504500 | 309.03 | 16.01 | 19.3x | same |
| DieHard/DieHard | 14 | 0.50 | 0.01 | 95.4x | same |
| DieHard/MCDieHarder | 14 | 0.33 | 0.00 | 66.5x | same |
| DieHard/MCDieHardest | 11566 | 0.61 | 0.03 | 18.7x | same |
| DiningPhilosophers/DiningPhilosophers | 58 | 0.36 | 0.01 | 71.5x | same |
| DiningPhilosophers/DiningPhilosophersPCal | 67 | 0.40 | 0.00 | 84.4x | same |
| DiningPhilosophers/DiningPhilosophers_anim | 58 | 0.40 | 0.01 | 42.3x | same |
| DiningPhilosophers/DiningPhilosophers_stats |  | 0.36 | 0.01 | 25.0x | same |
| Disruptor/MCDisruptor_MPMC | 112929 | 2.59 | 0.57 | 4.5x | same |
| Disruptor/MCDisruptor_MPMC_liveliness | 14365 | 1.67 | 0.30 | 5.6x | same |
| Disruptor/MCDisruptor_SPMC | 8496 | 0.96 | 0.16 | 5.9x | same |
| FiniteMonotonic/MCCRDT | 25000 | 9.10 | 1.08 | 8.4x | same |
| FiniteMonotonic/MCDistributedReplicatedLog | 37 | 0.50 | 0.01 |  | differs |
| FiniteMonotonic/MCReplicatedLog | 1363 | 0.49 | 0.30 | 1.6x | same |
| FlashProtocol/MCFlashWithMutex | 2024 | 0.55 | 0.03 | 16.5x | same |
| FlashProtocol/MCFlashWithMutexLive | 8096 | 8.89 | 0.17 | 52.9x | same |
| GameOfLife/GameOfLife | 65536 | 8.32 | 2.57 | 3.2x | same |
| GermanProtocol/MCGermanCMPWithMutex | 1107 | 0.39 | 0.01 | 37.9x | same |
| GermanProtocol/MCGermanControl | 735 | 0.38 | 0.01 | 37.1x | same |
| GermanProtocol/MCGermanData | 852 | 0.44 | 0.01 | 37.6x | same |
| Huang/Huang | 81256 | 34.59 | 37.73 | 0.9x | same |
| KeyValueStore/MCKVSSafetyMedium | 17220672 | 769.29 | 212.40 | 3.6x | same |
| KeyValueStore/MCKVSSafetySmall | 3409605 | 114.57 | 26.89 | 4.3x | same |
| KeyValueStore/MCKVsnap | 32293 | 7.18 | 1.59 | 4.5x | same |
| KnuthYao/SimKnuthYao |  | 0.38 | 0.16 | 2.4x | same |
| LearnProofs/MCFindHighest | 742 | 0.44 | 0.02 | 23.8x | same |
| LeastCircularSubstring/MCLeastCircularSubstringMedium | 1007232 | 179.43 | 1.54 | 116.3x | same |
| LeastCircularSubstring/MCLeastCircularSubstringSmall | 8554 | 0.60 | 0.02 | 38.3x | same |
| LoopInvariance/MCBinarySearch | 27953 | 1.88 | 0.37 | 5.0x | same |
| LoopInvariance/MCQuicksort | 4548 | 4.50 | 0.52 | 8.7x | same |
| Majority/MCMajority | 2733 | 0.49 | 0.02 | 26.9x | same |
| MisraReachability/MCParReach | 393 | 9.30 | 0.01 | 940.0x | same |
| MisraReachability/MCReachabilityTestAllGraphs | 0 | 2.27 | 0.83 | 2.7x | same |
| MisraReachability/MCReachabilityTestRandomGraphs | 0 | 0.96 | 0.30 | 3.2x | same |
| MisraReachability/MCReachable | 8 | 57.09 | 0.01 | 5685.9x | same |
| MissionariesAndCannibals/MissionariesAndCannibals | 61 | 0.34 | 0.00 | 69.8x | same |
| Moving_Cat_Puzzle/CatEvenBoxes | 48 | 0.32 | 0.00 | 66.3x | same |
| Moving_Cat_Puzzle/CatOddBoxes | 30 | 0.32 | 0.00 | 65.9x | same |
| MultiCarElevator/ElevatorLivenessMedium | 4122 | 9.28 | 0.04 | 225.1x | same |
| MultiCarElevator/ElevatorSafetyLarge |  | 900.06 | 737.43 |  | TLC over 900 s |
| MultiCarElevator/ElevatorSafetyMedium | 17997111 | 502.32 | 150.95 | 3.3x | same |
| MultiCarElevator/ElevatorSafetySmall | 4122 | 0.75 | 0.03 | 22.6x | same |
| MultiPaxos-SMR/MultiPaxos_MC |  | 900.02 |  |  | TLC over 900 s |
| MultiPaxos-SMR/MultiPaxos_MC_small | 343796 | 18.22 | 5.19 | 3.5x | same |
| N-Queens/Queens.toolbox/FourQueens/MC | 40 | 0.34 | 0.00 | 71.4x | same |
| N-Queens/QueensPluscal.toolbox/FourQueens/MC | 40 | 0.35 | 0.01 | 67.8x | same |
| NanoBlockchain/MCNanoMedium | 530587 | 14.69 | 4.44 | 3.3x | same |
| NanoBlockchain/MCNanoSmall | 3003 | 0.56 | 0.03 | 16.3x | same |
| Paxos/MCConsensus | 4 | 0.36 | 0.01 | 35.1x | same |
| Paxos/MCPaxos | 25 | 0.49 | 0.01 | 51.8x | same |
| Paxos/MCVoting | 77 | 0.50 | 0.01 | 49.7x | same |
| PaxosHowToWinATuringAward/MCConsensus | 4 | 0.35 | 0.01 | 34.3x | same |
| PaxosHowToWinATuringAward/MCPaxosSmall | 185369 | 97.71 | 16.08 | 6.1x | same |
| PaxosHowToWinATuringAward/MCPaxosTiny | 3921 | 1.71 | 0.20 | 8.4x | same |
| PaxosHowToWinATuringAward/MCVoting | 6752 | 1.86 | 0.20 | 9.2x | same |
| Prisoners/Prisoners | 214 | 0.36 | 0.00 | 73.0x | same |
| Prisoners_Single_Switch/Prisoner | 16 | 0.33 | 0.00 | 67.6x | same |
| Prisoners_Single_Switch/PrisonerLightUnknown | 62 | 0.33 | 0.00 | 66.7x | same |
| Prisoners_Single_Switch/PrisonerSolo | 2 | 0.33 | 0.00 | 67.5x | same |
| Prisoners_Single_Switch/PrisonerSoloLightUnknown | 4 | 0.33 | 0.00 | 67.8x | same |
| ReadersWriters/MC | 21527 | 1.68 | 0.33 | 5.1x | same |
| SDP_Verification/SDP_Attack_New_Solution_Spec/MC | 2881 | 1.22 | 0.16 | 7.6x | same |
| SDP_Verification/SDP_Attack_Spec/MC | 526 | 0.70 | 0.01 | 59.8x | same |
| SimplifiedFastPaxos/FastPaxos | 25617 | 30.29 | 2.71 | 11.2x | same |
| SimplifiedFastPaxos/Paxos | 1207 | 1.16 | 0.12 | 9.8x | same |
| SingleLaneBridge/MC | 3605 | 3.06 | 0.06 | 54.5x | same |
| SlidingPuzzles/SlidingPuzzles | 24005 | 1.29 | 0.29 | 4.4x | same |
| SlidingPuzzles/SlidingPuzzles_anim | 24005 | 1.60 | 0.33 | 4.8x | same |
| SlushProtocol/SlushMedium | 9886336 | 175.85 | 39.78 | 4.4x | same |
| SlushProtocol/SlushSmall | 274678 | 5.59 | 1.10 | 5.1x | same |
| SpanningTree/SpanTree | 1236 | 0.66 | 0.01 | 65.0x | same |
| SpanningTree/SpanTreeRandom | 65396 | 14.47 | 0.57 |  | differs |
| SpanningTree/SpanTreeTest4Nodes | 3984 | 0.76 | 0.03 | 22.6x | same |
| SpecificationRefinement/abstract | 4 | 0.32 | 0.00 | 65.7x | same |
| SpecificationRefinement/bad | 12 | 0.32 | 0.00 | 65.1x | same |
| SpecificationRefinement/good | 9 | 0.33 | 0.00 | 67.7x | same |
| SpecifyingSystems/AdvancedExamples/MCInnerSequential | 3528 | 1.02 | 0.04 | 26.6x | same |
| SpecifyingSystems/AdvancedExamples/MCInnerSerial | 195 | 369.52 | 223.79 | 1.7x | same |
| SpecifyingSystems/AsynchronousInterface/AsynchInterface | 12 | 0.31 | 0.01 | 51.2x | same |
| SpecifyingSystems/AsynchronousInterface/Channel | 12 | 0.32 | 0.01 | 56.1x | same |
| SpecifyingSystems/AsynchronousInterface/PrintValues | 0 | 0.33 | 0.01 | 56.5x | same |
| SpecifyingSystems/CachingMemory/MCInternalMemory | 4408 | 0.39 | 0.01 | 33.7x | same |
| SpecifyingSystems/CachingMemory/MCWriteThroughCache | 5196 | 0.63 | 0.06 | 10.0x | same |
| SpecifyingSystems/FIFO/MCInnerFIFO | 3864 | 0.37 | 0.01 | 31.1x | same |
| SpecifyingSystems/HourClock/HourClock | 12 | 0.32 | 0.01 | 56.5x | same |
| SpecifyingSystems/HourClock/HourClock2 | 12 | 0.33 | 0.01 | 56.3x | same |
| SpecifyingSystems/Liveness/LiveHourClock | 12 | 0.32 | 0.01 | 55.3x | same |
| SpecifyingSystems/Liveness/MCLiveInternalMemory | 4408 | 0.87 | 0.06 | 13.9x | same |
| SpecifyingSystems/Liveness/MCLiveWriteThroughCache | 5196 | 1.34 | 0.22 | 6.1x | same |
| SpecifyingSystems/RealTime/MCRealTimeHourClock | 216 | 0.35 | 0.01 | 59.7x | same |
| SpecifyingSystems/SimpleMath/SimpleMath | 0 | 0.32 | 0.01 | 55.4x | same |
| SpecifyingSystems/TLC/ABCorrectness | 20 | 0.32 | 0.01 | 60.3x | same |
| SpecifyingSystems/TLC/MCAlternatingBit | 240 | 0.43 | 0.01 | 74.3x | same |
| StaleViewShrink/StaleShrink | 16848 | 0.64 | 0.04 | 15.4x | same |
| StaleViewShrink/StaleShrinkV6Wait2x | 43348 | 0.88 | 0.10 | 8.9x | same |
| StaleViewShrink/StaleShrinkV3Cascade | 32551 | 0.74 | 0.08 | 9.7x | same |
| StaleViewShrink/StaleShrinkV6TooShort | 2132 | 0.44 | 0.01 | 42.9x | same |
| Stones/Stones | 0 | 0.40 | 0.01 | 33.7x | same |
| TLC/TestGraphs | 2590 | 0.93 | 0.05 | 18.0x | same |
| TeachingConcurrency/Simple | 723 | 0.36 | 0.01 | 62.5x | same |
| TeachingConcurrency/SimpleRegular | 277726 | 3.50 | 0.82 | 4.3x | same |
| TransitiveClosure/TransitiveClosure | 0 | 0.66 | 0.11 | 5.8x | same |
| TwoPhase/MCTwoPhase | 4 | 0.33 | 0.01 | 62.5x | same |
| YoYo/MCYoYoNoPruning | 60 | 0.49 | 0.01 | 39.8x | same |
| YoYo/MCYoYoPruning | 102 | 0.54 | 0.01 | 45.2x | same |
| YoYo/YoYoAllGraphs | 26731 | 3.97 | 0.55 | 7.2x | same |
| aba-asyn-byz/aba_asyn_byz |  | 900.13 | 34.24 |  | TLC over 900 s |
| acp/ACP_NB_TLC | 4284 | 1.09 | 0.03 | 38.3x | same |
| acp/ACP_NB_WRONG_TLC | 11852 | 0.61 | 0.04 | 15.0x | same |
| acp/ACP_SB_TLC | 54944 | 0.97 | 0.10 | 10.1x | same |
| allocator/AllocatorImplementation | 17701 | 5.50 | 1.01 | 5.5x | same |
| allocator/AllocatorRefinement | 1690 | 0.75 | 0.09 | 8.8x | same |
| allocator/SchedulingAllocator | 1690 | 1.74 | 0.04 | 48.7x | same |
| allocator/SimpleAllocator | 400 | 0.87 | 0.01 | 59.9x | same |
| barriers/Barrier | 64 | 0.32 | 0.01 | 58.0x | same |
| barriers/Barriers | 29279 | 2.46 | 0.42 | 5.9x | same |
| bcastByz/bcastByz | 14424 | 9.86 | 0.35 | 28.5x | same |
| bcastByz/bcastByzNoBcast | 45 | 0.61 | 0.01 | 52.4x | same |
| bosco/bosco | 1072452 | 555.57 | 163.41 | 3.4x | same |
| braf/BufferedRandomAccessFile | 3316 | 36.29 | 9.56 | 3.8x | same |
| btree/MCbtree | 374727 | 28.52 | 4.57 | 6.2x | same |
| btree/kvstore | 2641 | 0.43 | 0.01 | 42.2x | same |
| byihive/VoucherCancel | 4199 | 0.54 | 0.08 | 7.2x | same |
| byihive/VoucherIssue | 4199 | 0.66 | 0.08 | 8.6x | same |
| byihive/VoucherLifeCycle | 64 | 0.32 | 0.01 | 63.5x | same |
| byihive/VoucherRedeem | 4199 | 0.54 | 0.08 | 7.2x | same |
| byihive/VoucherTransfer | 4197 | 0.54 | 0.08 | 7.2x | same |
| byzpaxos/Consensus | 4 | 0.38 | 0.00 | 75.9x | same |
| byzpaxos/PConProof |  | 900.14 | 180.30 |  | TLC over 900 s |
| byzpaxos/VoteProof | 6962 | 8.18 | 1.08 | 7.6x | same |
| cf1s-folklore/cf1s_folklore | 2057174 | 154.09 | 7.43 | 20.7x | same |
| chang_roberts/MCChangRoberts | 137 | 0.36 | 0.01 | 71.9x | same |
| dag-consensus/BlockDagTest | 0 | 0.32 | 0.00 | 65.7x | same |
| dag-consensus/TLCSailfish1 | 109604 | 34.46 | 9.60 | 3.6x | same |
| dijkstra-mutex/DijkstraMutex.toolbox/LSpec-model/MC | 90882 | 16.92 | 0.52 | 32.6x | same |
| dijkstra-mutex/DijkstraMutex.toolbox/Safety-4-processors/MC | 33288512 | 271.96 | 97.94 | 2.8x | same |
| echo/MCEcho | 75 | 0.55 | 0.01 | 89.7x | same |
| ewd426/SimTokenRing |  | 3.12 | 1.02 | 3.1x | same |
| ewd426/TokenRing | 46656 | 1.40 | 0.14 | 10.3x | same |
| ewd687a/EWD687a_anim |  | 1.02 | 0.01 |  | differs |
| ewd687a/MCEWD687a | 18028 | 3.44 | 0.33 | 10.5x | same |
| ewd840/EWD840 | 302 | 0.50 | 0.01 | 83.7x | same |
| ewd840/EWD840_anim |  | 0.76 | 0.06 | 12.1x | same |
| ewd840/EWD840_json | 1566 | 0.43 | 0.01 | 36.4x | same |
| ewd840/SyncTerminationDetection | 129 | 0.35 | 0.01 | 62.0x | same |
| ewd998/AsyncTerminationDetection | 4097 | 0.60 | 0.02 | 31.2x | same |
| ewd998/EWD998Small | 1520618 | 22.62 | 10.62 | 2.1x | same |
| ewd998/EWD998Chan | 1524022 | 674.80 | 110.30 | 6.1x | same |
| ewd998/EWD998ChanID | 14 | 0.66 | 0.03 | 25.9x | same |
| ewd998/EWD998ChanTrace |  | 0.44 | 0.01 | 37.1x | same |
| ewd998/EWD998PCal | 321370 | 27.04 | 7.06 | 3.8x | same |
| ewd998/SmokeEWD998 |  | 0.87 | 0.17 | 5.0x | same |
| glowingRaccoon/clean | 63 | 0.33 | 0.01 | 56.2x | same |
| glowingRaccoon/product | 305 | 0.43 | 0.01 | 29.9x | same |
| glowingRaccoon/stages | 83 | 0.35 | 0.01 | 61.2x | same |
| lamport_mutex/MCLamportMutex | 724274 | 10.57 | 3.22 | 3.3x | same |
| locks_auxiliary_vars/Lock | 12 | 0.33 | 0.01 | 56.0x | same |
| locks_auxiliary_vars/LockHS | 28 | 0.44 | 0.01 | 37.4x | same |
| locks_auxiliary_vars/Peterson | 42 | 0.35 | 0.01 | 62.2x | same |
| nbacc_ray97/nbacc_ray97 | 3016 | 0.54 | 0.02 | 33.6x | same |
| nbacg_guer01/nbacg_guer01 | 24922 | 2.39 | 0.08 | 29.2x | same |
| spanning/MC_spanning | 5 | 0.33 | 0.01 | 56.4x | same |
| sums_even/MC_sums_even | 0 | 0.50 | 0.05 | 10.0x | same |
| tcp/MCtcp | 1182 | 0.61 | 0.02 | 38.0x | same |
| tower_of_hanoi/Hanoi.toolbox/Model_1/MC | 233 | 0.36 | 0.01 |  | differs |
| tower_of_hanoi/Hanoi_anim | 25 | 0.49 | 0.01 | 42.2x | same |
| transaction_commit/2PCwithBTM | 1245 | 0.37 | 0.01 | 31.2x | same |
| transaction_commit/PaxosCommit | 1321761 | 122.44 | 20.10 | 6.1x | same |
| transaction_commit/TCommit | 34 | 0.33 | 0.01 | 56.4x | same |
| transaction_commit/TwoPhase | 288 | 0.32 | 0.01 | 57.1x | same |
