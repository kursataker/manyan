/* tpp: a TLA+ model checker. See docs/manual.md. */
#ifndef TPP_H
#define TPP_H

#define TPP_VERSION "0.1.0"

/* Exit statuses, as TLC reports them (docs/standards/tla2.md, 11.3). */
enum {
    TPP_OK = 0,
    TPP_VIOLATION_ASSUMPTION = 10,
    TPP_VIOLATION_DEADLOCK = 11,
    TPP_VIOLATION_SAFETY = 12,
    TPP_VIOLATION_LIVENESS = 13,
    TPP_VIOLATION_ASSERT = 14,
    TPP_FAILURE_EVAL = 75,
    TPP_FAILURE_SAFETY_EVAL = 76,
    TPP_ERROR_PARSE = 150,
    TPP_ERROR_CONFIG = 151,
    TPP_ERROR = 255
};

typedef struct {
    const char *spec;    /* path of the root module, with or without .tla */
    const char *config;  /* path of the model configuration; NULL for SPEC.cfg */
    int check_deadlock;  /* 0 turns deadlock checking off, as -deadlock does */
    const char *dump;    /* write the state graph in dot format to this file, or NULL */
    int workers;         /* processes expanding states; 0 or 1 for one */
    int simulate;        /* random behaviors instead of the state graph, as -simulate */
    long traces;         /* simulation: behaviors to make, 0 for no end (-simulate num=N) */
    long depth;          /* simulation: states in a behavior, 0 for 100 (-depth) */
    long long seed;      /* simulation: the seed, when seed_given (-seed) */
    int seed_given;
    long long aril;      /* simulation: draws to skip after seeding (-aril) */
} TPPOptions;

/* Check the model and print TLC's report to standard output. Returns an
   exit status. */
int tpp_check(const TPPOptions *options);

#endif
