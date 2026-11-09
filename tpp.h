/* tpp: a TLA+ model checker. See docs/manual.md. */
#ifndef TPP_H
#define TPP_H

#include <stddef.h>
#include <stdint.h>

#define TPP_VERSION "0.6.1"

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
    TPP_FAILURE_LIVENESS_EVAL = 77,
    TPP_ERROR_PARSE = 150,
    TPP_ERROR_CONFIG = 151,
    TPP_ERROR = 255
};

typedef struct {
    const char *spec;    /* path of the root module, with or without .tla */
    const char *config;  /* path of the model configuration; NULL for SPEC.cfg */
    int check_deadlock;  /* 0 turns deadlock checking off, as -deadlock does */
    const char *dump;    /* write the state graph in dot format to this file, or NULL */
    int dump_actions;    /* label the dump's steps with their actions (-dump dot,actionlabels) */
    int nowarning;       /* TLC's warnings off, as -nowarning */
    int difftrace;       /* print only what each step of a trace changes, as -difftrace */
    int view;            /* a trace shows each state as the VIEW's value, as -view */
    int keep_going;      /* report each violated invariant and action property, and go on (-continue) */
    const char *user_file;   /* write Print's output to this file instead, or NULL (-userFile) */
    int workers;         /* processes expanding states; 0 or 1 for one */
    int simulate;        /* random behaviors instead of the state graph, as -simulate */
    long traces;         /* simulation: behaviors to make, 0 for no end (-simulate num=N) */
    long depth;          /* simulation: states in a behavior, 0 for 100 (-depth) */
    long long seed;      /* the seed, when seed_given (-seed): a simulation's, and model checking's for RandomElement */
    int seed_given;
    long long aril;      /* simulation: draws to skip after seeding (-aril) */
    const char *trace_file;  /* simulation: write behavior N to FILE_0_N, or NULL (-simulate file=FILE) */
    const char *const *properties;  /* Java system properties, "k=v" each, NULL-terminated (-Dk=v) */
} TPPOptions;

/* Check the model and print TLC's report to standard output. Returns an
   exit status. */
int tpp_check(const TPPOptions *options);

/* == Native operators ==
   TLC computes some operators of its modules in Java instead of from
   their definitions. A native operator is that Java in C: tpp_define
   binds it to the operator NAME of every module named MODULE that a spec
   loads, by name, as TLC binds its Java, and only when the definition
   takes ARITY arguments. JAVA is the method as TLC names it in an error
   in it. ARITY TPP_LAZY makes it take its arguments unevaluated, as TLC's
   evaluation methods do, with tpp_arg; it may then give NULL, which
   leaves the call to the definition. tpp-std.c defines TLC's own modules
   and CommunityModules' with these (docs/std.md). Define before
   tpp_check; returns 0, or -1 when out of memory. */
typedef struct TPPValue TPPValue;
typedef struct TPPCall TPPCall;   /* the call being evaluated */
typedef TPPValue *(*TPPNative)(TPPCall *call, TPPValue **args);
#define TPP_LAZY (-1)
int tpp_define(const char *module, const char *name, int arity, TPPNative fn, const char *java);

/* Values live until the evaluation that made them ends; a native's
   arguments, as they are given, and what it makes. Sets and functions are
   given in TLC's order: sorted. */
typedef enum { TPP_BOOL, TPP_INT, TPP_STRING, TPP_MODEL, TPP_TUPLE, TPP_FUNCTION, TPP_SET, TPP_OTHER } TPPKind;
TPPKind tpp_kind(TPPValue *);   /* a record is a function; a tuple a function on 1..n */
TPPValue *tpp_bool(int);
TPPValue *tpp_int(int64_t);
TPPValue *tpp_string(const char *, size_t);
TPPValue *tpp_tuple(size_t n, TPPValue *const *elems);
TPPValue *tpp_function(size_t n, TPPValue *const *keys, TPPValue *const *values);   /* keys distinct, any order */
TPPValue *tpp_set(size_t n, TPPValue *const *elems);   /* any order, duplicates too */
int tpp_is_true(TPPValue *);
int64_t tpp_int_value(TPPValue *);
const char *tpp_string_value(TPPValue *, size_t *len);
/* A tuple's or function's size, keys and values; a set's elements, which
   tpp_elements gives it. */
size_t tpp_size(TPPValue *);
TPPValue *tpp_key(TPPValue *, size_t);
TPPValue *tpp_value(TPPValue *, size_t);
TPPValue *tpp_field(TPPValue *record, const char *name);   /* NULL when it has none */
TPPValue *tpp_elements(TPPValue *set);   /* enumerated, as TLC's toSetEnum; NULL for a set it cannot enumerate, or another value */
TPPValue *tpp_element(TPPValue *elements, size_t);
int tpp_compare(TPPValue *, TPPValue *);   /* TLC's order of values: <0, 0 or >0 */
const char *tpp_print(TPPValue *);         /* as TLC prints it */
TPPValue *tpp_arg(TPPCall *, int i);       /* a TPP_LAZY native's argument i, evaluated */
/* An error in the native, which TLC reports as one in its Java. */
#if defined(__GNUC__)
__attribute__((noreturn, format(printf, 2, 3)))
#endif
void tpp_fail(TPPCall *, const char *fmt, ...);

#endif
