# Symbolic benchmark

<!-- results -->

tlaplus/Examples at ca168055339d, the models its manifests check with Apalache, to length 5. Measured on arm64 Darwin 25.5.0, one run, wall-clock seconds.

46 models: 46 same. Where manyan gives Apalache's outcome, it is 273.9x as fast (geometric mean).

| model | outcome | Apalache | Manyan | speedup | result |
|---|---|---:|---:|---:|---|
| CigaretteSmokers/APCigaretteSmokers | NoError | 2.59 | 0.01 | 442.4x | same |
| CoffeeCan/APCoffeeCan | NoError | 1.76 | 0.03 | 69.9x | same |
| DieHard/APDieHarder | NoError | 3.50 | 0.04 | 96.2x | same |
| DiningPhilosophers/APDiningPhilosophersPCal | NoError | 6.53 | 0.01 | 1179.0x | same |
| Disruptor/APDisruptor_MPMC | NoError | 3.62 | 0.01 | 654.1x | same |
| Disruptor/APDisruptor_SPMC | NoError | 2.38 | 0.01 | 417.7x | same |
| EinsteinRiddle/Einstein | Error FindSolution 1 | 2.39 | 0.01 | 205.7x | same |
| FiniteMonotonic/APCRDT | NoError | 11.03 | 0.01 | 952.2x | same |
| FiniteMonotonic/APReplicatedLog | NoError | 1.89 | 0.01 | 337.5x | same |
| FlashProtocol/APFlashWithMutex | NoError | 36.73 | 0.37 | 100.1x | same |
| GermanProtocol/APGermanCMPWithMutex | NoError | 5.31 | 0.01 | 452.7x | same |
| GermanProtocol/APGermanControl | NoError | 2.15 | 0.01 | 183.9x | same |
| GermanProtocol/APGermanData | NoError | 2.95 | 0.01 | 255.2x | same |
| MisraReachability/APParReach | NoError | 2.38 | 0.01 | 204.0x | same |
| MissionariesAndCannibals/APMissionariesAndCannibals | NoError | 1.96 | 0.01 | 352.2x | same |
| Moving_Cat_Puzzle/APCat | NoError | 1.60 | 0.01 | 286.4x | same |
| ReadersWriters/APReadersWriters | NoError | 2.07 | 0.01 | 371.5x | same |
| SpanningTree/APSpanTree | ExecutionsTooShort | 1.75 | 0.01 | 151.2x | same |
| SpecifyingSystems/AsynchronousInterface/APAsynchInterface | NoError | 1.45 | 0.01 | 276.1x | same |
| SpecifyingSystems/AsynchronousInterface/APChannel | NoError | 1.55 | 0.01 | 277.2x | same |
| SpecifyingSystems/Composing/APChannel | NoError | 1.55 | 0.01 | 274.0x | same |
| SpecifyingSystems/Composing/APHourClock | NoError | 1.35 | 0.01 | 238.4x | same |
| SpecifyingSystems/FIFO/APChannel | NoError | 1.55 | 0.01 | 275.3x | same |
| SpecifyingSystems/FIFO/APInnerFIFO | NoError | 1.76 | 0.01 | 305.7x | same |
| SpecifyingSystems/FIFO/APInnerFIFOInstanced | NoError | 1.71 | 0.01 | 302.1x | same |
| SpecifyingSystems/FIFO/APMCInnerFIFO | NoError | 1.81 | 0.01 | 324.2x | same |
| SpecifyingSystems/HourClock/APHourClock | NoError | 1.36 | 0.01 | 244.8x | same |
| SpecifyingSystems/HourClock/APHourClock2 | NoError | 1.50 | 0.01 | 129.7x | same |
| SpecifyingSystems/Liveness/APHourClock | NoError | 1.45 | 0.01 | 254.2x | same |
| SpecifyingSystems/Liveness/APLiveHourClock | NoError | 1.45 | 0.01 | 258.6x | same |
| acp/APACP_SB | NoError | 3.31 | 0.01 | 293.4x | same |
| barriers/APBarrier | NoError | 2.08 | 0.01 | 373.7x | same |
| bcastFolklore/APbcastFolklore | NoError | 3.15 | 0.01 | 271.6x | same |
| bosco/APbosco | NoError | 14.98 | 0.05 | 312.6x | same |
| c1cs/APc1cs | NoError | 3.70 | 0.21 | 17.3x | same |
| chang_roberts/APChangRoberts | NoError | 6.30 | 0.01 | 1103.8x | same |
| ewd426/APTokenRing | NoError | 3.57 | 0.02 | 188.3x | same |
| ewd840/APEWD840 | NoError | 2.49 | 0.01 | 433.8x | same |
| ewd840/APSyncTerminationDetection | NoError | 2.40 | 0.01 | 205.3x | same |
| glowingRaccoon/APclean | NoError | 1.66 | 0.01 | 299.3x | same |
| glowingRaccoon/APstages | NoError | 1.73 | 0.01 | 308.9x | same |
| lamport_mutex/APLamportMutex | NoError | 9.57 | 0.01 | 811.7x | same |
| nbacg_guer01/APnbacg_guer01 | NoError | 5.86 | 0.09 | 68.1x | same |
| spanning/APspanning | NoError | 1.97 | 0.01 | 340.1x | same |
| tcp/APtcp | NoError | 13.31 | 0.02 | 534.2x | same |
| transaction_commit/APTCommit | NoError | 2.07 | 0.01 | 366.6x | same |
