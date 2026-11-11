# Symbolic benchmark

<!-- results -->

tlaplus/Examples at ca168055339d, the models its manifests check with Apalache, to length 5. Measured on arm64 Darwin 25.5.0, one run, wall-clock seconds.

46 models: 46 same. Where manyan gives Apalache's outcome, it is 278.8x as fast (geometric mean).

| model | outcome | Apalache | Manyan | speedup | result |
|---|---|---:|---:|---:|---|
| CigaretteSmokers/APCigaretteSmokers | NoError | 3.25 | 0.01 | 620.0x | same |
| CoffeeCan/APCoffeeCan | NoError | 1.89 | 0.02 | 94.5x | same |
| DieHard/APDieHarder | NoError | 3.88 | 0.04 | 97.0x | same |
| DiningPhilosophers/APDiningPhilosophersPCal | NoError | 7.66 | 0.01 | 764.2x | same |
| Disruptor/APDisruptor_MPMC | NoError | 4.00 | 0.01 | 404.1x | same |
| Disruptor/APDisruptor_SPMC | NoError | 2.58 | 0.01 | 255.9x | same |
| EinsteinRiddle/Einstein | Error FindSolution 1 | 2.74 | 0.02 | 139.9x | same |
| FiniteMonotonic/APCRDT | NoError | 12.96 | 0.02 | 648.3x | same |
| FiniteMonotonic/APReplicatedLog | NoError | 2.21 | 0.01 | 221.8x | same |
| FlashProtocol/APFlashWithMutex | NoError | 41.99 | 0.40 | 106.1x | same |
| GermanProtocol/APGermanCMPWithMutex | NoError | 5.72 | 0.01 | 564.2x | same |
| GermanProtocol/APGermanControl | NoError | 5.02 | 0.01 | 493.4x | same |
| GermanProtocol/APGermanData | NoError | 3.54 | 0.02 | 175.4x | same |
| MisraReachability/APParReach | NoError | 2.87 | 0.01 | 287.1x | same |
| MissionariesAndCannibals/APMissionariesAndCannibals | NoError | 2.40 | 0.04 | 65.8x | same |
| Moving_Cat_Puzzle/APCat | NoError | 1.94 | 0.01 | 197.2x | same |
| ReadersWriters/APReadersWriters | NoError | 2.49 | 0.01 | 262.0x | same |
| SpanningTree/APSpanTree | ExecutionsTooShort | 2.03 | 0.01 | 205.8x | same |
| SpecifyingSystems/AsynchronousInterface/APAsynchInterface | NoError | 1.65 | 0.00 | 332.7x | same |
| SpecifyingSystems/AsynchronousInterface/APChannel | NoError | 1.78 | 0.00 | 368.6x | same |
| SpecifyingSystems/Composing/APChannel | NoError | 1.78 | 0.00 | 367.3x | same |
| SpecifyingSystems/Composing/APHourClock | NoError | 1.58 | 0.00 | 322.8x | same |
| SpecifyingSystems/FIFO/APChannel | NoError | 1.73 | 0.01 | 341.6x | same |
| SpecifyingSystems/FIFO/APInnerFIFO | NoError | 2.06 | 0.01 | 400.7x | same |
| SpecifyingSystems/FIFO/APInnerFIFOInstanced | NoError | 2.10 | 0.01 | 211.9x | same |
| SpecifyingSystems/FIFO/APMCInnerFIFO | NoError | 2.08 | 0.01 | 206.1x | same |
| SpecifyingSystems/HourClock/APHourClock | NoError | 1.53 | 0.01 | 302.9x | same |
| SpecifyingSystems/HourClock/APHourClock2 | NoError | 1.53 | 0.01 | 150.0x | same |
| SpecifyingSystems/Liveness/APHourClock | NoError | 1.53 | 0.01 | 305.2x | same |
| SpecifyingSystems/Liveness/APLiveHourClock | NoError | 1.55 | 0.01 | 303.9x | same |
| acp/APACP_SB | NoError | 3.94 | 0.01 | 383.7x | same |
| barriers/APBarrier | NoError | 2.07 | 0.01 | 400.3x | same |
| bcastFolklore/APbcastFolklore | NoError | 3.55 | 0.01 | 350.2x | same |
| bosco/APbosco | NoError | 16.53 | 0.05 | 313.9x | same |
| c1cs/APc1cs | NoError | 4.82 | 0.19 | 25.1x | same |
| chang_roberts/APChangRoberts | NoError | 7.90 | 0.01 | 772.0x | same |
| ewd426/APTokenRing | NoError | 4.20 | 0.02 | 207.5x | same |
| ewd840/APEWD840 | NoError | 3.29 | 0.01 | 318.7x | same |
| ewd840/APSyncTerminationDetection | NoError | 3.35 | 0.01 | 331.0x | same |
| glowingRaccoon/APclean | NoError | 2.29 | 0.01 | 233.2x | same |
| glowingRaccoon/APstages | NoError | 2.23 | 0.00 | 469.4x | same |
| lamport_mutex/APLamportMutex | NoError | 13.90 | 0.01 | 1430.7x | same |
| nbacg_guer01/APnbacg_guer01 | NoError | 7.65 | 0.08 | 99.8x | same |
| spanning/APspanning | NoError | 3.38 | 0.01 | 659.3x | same |
| tcp/APtcp | NoError | 17.65 | 0.04 | 456.3x | same |
| transaction_commit/APTCommit | NoError | 2.68 | 0.01 | 264.3x | same |
