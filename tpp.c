/* tpp: a TLA+ model checker, with TLC as its oracle.

   The language and the results it must produce are defined by
   docs/standards/tla2.md; section numbers below (§) refer to it.

   A run reads the root module and the modules it uses (lexer, parser),
   resolves names (resolver), binds the constants from the configuration,
   and explores the state graph breadth first (checker), evaluating
   expressions (evaluator) and enumerating the states an initial predicate
   or an action allows (generator). Worker processes may expand the
   states of a level in parallel; the search merges what they find in
   order, so the result is the same with any number of them. */
#define _DEFAULT_SOURCE
#define _DARWIN_C_SOURCE
#include "tpp.h"

#include <ctype.h>
#include <math.h>
#include <sched.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct Value Value;
typedef struct Expr Expr;
typedef struct Def Def;
typedef struct Module Module;
typedef struct Env Env;
typedef struct Inst Inst;

/* ---------------------------------------------------------------- memory */

/* Values and expressions live in arenas. The permanent arena holds the
   modules and everything computed before the search; the scratch arena
   holds what the search computes for one state and is reset after it. */
typedef struct Chunk { struct Chunk *next; size_t used, size; char data[]; } Chunk;
typedef struct { Chunk *head; } Arena;

static Arena perm_arena, scratch_arena;
static Arena *arena = &perm_arena;

static void *xmalloc(size_t n) {
    void *p = malloc(n ? n : 1);
    if (!p) { fputs("tpp: out of memory\n", stderr); exit(TPP_ERROR); }
    return p;
}

static void *xrealloc(void *p, size_t n) {
    p = realloc(p, n ? n : 1);
    if (!p) { fputs("tpp: out of memory\n", stderr); exit(TPP_ERROR); }
    return p;
}

/* Memory the caller fills; alloc's is zeroed. */
static void *alloc_raw(size_t n) {
    n = (n + 15) & ~(size_t)15;
    Chunk *c = arena->head;
    if (!c || c->used + n > c->size) {
        size_t size = n > 1 << 20 ? n : 1 << 20;
        Chunk *fresh = xmalloc(sizeof(Chunk) + size);

        fresh->next = c; fresh->used = 0; fresh->size = size;
        arena->head = c = fresh;
    }
    void *p = c->data + c->used;
    c->used += n;
    return p;
}

static void *alloc(size_t n) { return memset(alloc_raw(n), 0, n); }

#define NEW(T) ((T *)alloc(sizeof(T)))
#define NEWN(T, n) ((T *)alloc(sizeof(T) * (size_t)(n)))

/* v, of n elements of the size given, with room for one more: it holds 8,
   then doubles. */
static void *grow(void *v, int n, size_t size) {
    if (n && (n < 8 || n & (n - 1))) return v;
    void *g = alloc_raw(size * (size_t)(n ? 2 * n : 8));
    if (n) memcpy(g, v, size * (size_t)n);
    return g;
}
#define PUSH(v, n, x) ((v) = grow((v), (n), sizeof *(v)), (v)[(n)++] = (x))

static unsigned scratch_resets;   /* a value's address names it only between two */

static void arena_reset(Arena *a) {
    scratch_resets += a == &scratch_arena;
    Chunk *c = a->head;
    while (c && c->next) { Chunk *next = c->next; free(c); c = next; }
    if (c) c->used = 0;
    a->head = c;
}

static void arena_free(Arena *a) {
    arena_reset(a);
    free(a->head);
    a->head = NULL;
}

static size_t arena_bytes(Arena *a) {
    size_t n = 0;
    for (Chunk *c = a->head; c; c = c->next) n += c->used;
    return n;
}

/* What one iteration of a loop computing a constant made, from where its
   arena stood: dropped after it, unless a value made before it was given
   one it made. Neither arena of the search has them: the permanent one
   keeps what is computed once, and the scratch one is dropped by state. */
typedef struct { Arena *a; Chunk *c; size_t used; int kept; } Region;
static Region regions[64];
static int nregions;

static int in_region(Region *r, void *p) {
    for (Chunk *c = r->a->head; c; c = c->next) {
        char *from = c->data + (c == r->c ? r->used : 0);
        if ((char *)p >= from && (char *)p < c->data + c->used) return 1;
        if (c == r->c) return 0;
    }
    return 0;
}

/* holder, a value or a binding, is given a value it keeps. */
static void held_by(void *holder) {
    for (int i = nregions - 1; i >= 0 && !in_region(&regions[i], holder); i--) regions[i].kept = 1;
}

static Region *region_open(void) {
    if (arena == &perm_arena || arena == &scratch_arena || nregions == 64) return NULL;
    Region *r = &regions[nregions++];
    *r = (Region){arena, arena->head, arena->head ? arena->head->used : 0, 0};
    return r;
}

static void region_close(Region *r) {
    if (!r) return;
    nregions--;
    if (r->kept) return;
    while (r->a->head != r->c) { Chunk *next = r->a->head->next; free(r->a->head); r->a->head = next; }
    if (r->c) r->c->used = r->used;
}

/* A growable byte buffer. */
typedef struct { char *s; size_t n, cap; } Buf;

static void buf_add(Buf *b, const char *s, size_t n) {
    if (b->n + n + 1 > b->cap) {
        b->cap = (b->n + n + 1) * 2;
        b->s = xrealloc(b->s, b->cap);
    }
    memcpy(b->s + b->n, s, n);
    b->n += n;
    b->s[b->n] = 0;
}

static void buf_puts(Buf *b, const char *s) { buf_add(b, s, strlen(s)); }

static void buf_printf(Buf *b, const char *fmt, ...) {
    char small[256];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(small, sizeof small, fmt, ap);
    va_end(ap);
    if (n < (int)sizeof small) { buf_add(b, small, (size_t)n); return; }
    char *big = xmalloc((size_t)n + 1);
    va_start(ap, fmt);
    vsnprintf(big, (size_t)n + 1, fmt, ap);
    va_end(ap);
    buf_add(b, big, (size_t)n);
    free(big);
}

static char *buf_take(Buf *b) {
    char *s = alloc(b->n + 1);
    if (b->n) memcpy(s, b->s, b->n);
    free(b->s);
    b->s = NULL; b->n = b->cap = 0;
    return s;
}

/* ---------------------------------------------------------------- strings */

/* Every identifier and string is interned; strings compare by the order in
   which they were first interned (§7.5). */
typedef struct { char *s; int len; Value *value, *model; } Str;   /* its V_STR and V_MODEL, once made */

static Str *strs;
static int nstrs, capstrs;
static int *str_table;   /* open addressing: index + 1, 0 empty */
static int str_table_size;

static uint64_t hash_bytes(const void *p, size_t n, uint64_t h) {
    const unsigned char *s = p;
    for (size_t i = 0; i < n; i++) { h ^= s[i]; h *= 0x100000001b3ULL; }
    return h;
}

static int in_worker;   /* this process expands states for the search: it changes no shared data */
static int exit_requested;   /* TLCSet("exit", TRUE) */
static _Noreturn void worker_cannot(void);

static int intern_n(const char *s, int len) {
    if (nstrs * 2 >= str_table_size && !in_worker) {
        int size = str_table_size ? str_table_size * 2 : 1024;
        int *table = calloc((size_t)size, sizeof(int));
        for (int i = 0; i < nstrs; i++) {
            uint64_t h = hash_bytes(strs[i].s, (size_t)strs[i].len, 0xcbf29ce484222325ULL);
            int j = (int)(h & (uint64_t)(size - 1));
            while (table[j]) j = (j + 1) & (size - 1);
            table[j] = i + 1;
        }
        free(str_table);
        str_table = table;
        str_table_size = size;
    }
    uint64_t h = hash_bytes(s, (size_t)len, 0xcbf29ce484222325ULL);
    int j = (int)(h & (uint64_t)(str_table_size - 1));
    while (str_table[j]) {
        Str *t = &strs[str_table[j] - 1];
        if (t->len == len && memcmp(t->s, s, (size_t)len) == 0) return str_table[j] - 1;
        j = (j + 1) & (str_table_size - 1);
    }
    if (in_worker) worker_cannot();   /* a new string gets its number from the search */
    if (nstrs == capstrs) {
        capstrs = capstrs ? capstrs * 2 : 1024;
        strs = xrealloc(strs, sizeof(Str) * (size_t)capstrs);
    }
    char *copy = xmalloc((size_t)len + 1);
    memcpy(copy, s, (size_t)len);
    copy[len] = 0;
    strs[nstrs].s = copy;
    strs[nstrs].len = len;
    strs[nstrs].value = strs[nstrs].model = NULL;
    str_table[j] = nstrs + 1;
    return nstrs++;
}

static int intern(const char *s) { return intern_n(s, (int)strlen(s)); }
#define S(id) (strs[id].s)

/* Names TLC interns before it reads a module, in its order. */
static const char *builtin_names[] = {
    "'", "*", "-", "+", "=", "\\lnot", "~", "\\neg", "\\land", "/\\", "\\lor", "\\/", "~>", "=>",
    "[]", "<>", "ENABLED", "UNCHANGED", "SUBSET", "UNION", "DOMAIN", "\\cdot", "\\equiv", "<=>",
    "-+->", "/=", "#", "\\subseteq", "\\in", "\\notin", "\\times", "\\", "\\intersect", "\\cap",
    "\\union", "\\cup", "...", "..", "$", "@@", ":>", "\\o", "\\circ", "STRING", "FALSE", "TRUE",
    "BOOLEAN", "$AngleAct", "$BoundedChoose", "$BoundedExists", "$BoundedForall", "$CartesianProd",
    "$Case", "$ConjList", "$DisjList", "$Except", "$FcnApply", "$FcnConstructor", "$IfThenElse",
    "$NonRecursiveFcnSpec", "$Pair", "$RcdConstructor", "$RcdSelect", "$RecursiveFcnSpec", "$Seq",
    "$SetEnumerate", "$SetOfAll", "$SetOfFcns", "$SetOfRcds", "$SF", "$SquareAct", "$SubsetOf",
    "$TemporalExists", "$TemporalForall", "$TemporalWhile", "$Tuple", "$UnboundedChoose",
    "$UnboundedExists", "$UnboundedForall", "$WF", "$Nop", "$Qed", "$Pfcase", "$Have", "$Take",
    "$Pick", "$Witness", "$Suffices", "@", "VARIABLE", "<<", "SF_", "WF_", "LAMBDA", NULL};

/* ---------------------------------------------------------------- errors */

/* An error unwinds to the innermost handler with its message. */
typedef struct Expr Expr;
static sigjmp_buf *handler;
static Buf error_message;
static Expr *error_expr;   /* the expression an evaluation error names, if any */
static int error_status;   /* the exit status the error implies */
/* What TLC raises the error as: 0, its evaluator's RuntimeException; 1, an
   EvalException of a standard module's operator; 2, a TLCRuntimeException,
   with the exit status of its error code. */
static int error_module;
static int stackless;         /* TLC's error call stack is empty: 1, or 2 for its StackOverflowError */
/* The Java methods of the standard operators being applied, innermost last.
   TLC wraps any error inside one, but an EvalException, in a preamble. */
static const char *java_frames[64];
static int java_depth;
/* The values TLC made with a source being worked on, outermost first: the
   function constructors computed in full, and set operations it keeps as
   their definitions. An error raised inside one is one TLC cannot
   fingerprint (§11.3). */
static Expr *fp_trace[64];
static int fp_depth;

static void fp_enter(Expr *where) {
    if (!where) return;
    if (fp_depth < 64) fp_trace[fp_depth] = where;
    fp_depth++;
}
static void fp_leave(Expr *where) { fp_depth -= where != NULL; }

/* The source of a value made at e: none before the search, as TLC
   computes the definitions of constants without one. */
static Expr *source(Expr *e) { return arena == &scratch_arena ? e : NULL; }

/* TLC's exit status for an error evaluating the spec. */
static int eval_failure_status(void) { return error_status == TPP_VIOLATION_ASSERT ? TPP_VIOLATION_ASSERT : TPP_FAILURE_EVAL; }

static void error_reset(void) {
    error_message.n = 0;
    if (error_message.s) error_message.s[0] = 0;
    error_expr = NULL;
    error_status = TPP_ERROR;
    error_module = 0;
    stackless = 0;
}

static _Noreturn void raise_error(void) {
    if (!handler) {
        fputs(error_message.s ? error_message.s : "error", stdout);
        exit(TPP_ERROR);
    }
    siglongjmp(*handler, 1);
}

static void loc_line(Buf *b, Expr *e);

static _Noreturn void vfail(Expr *e, int module, int status, const char *fmt, va_list ap) {
    va_list size;
    va_copy(size, ap);
    int n = vsnprintf(NULL, 0, fmt, size);
    va_end(size);
    char *small = xmalloc((size_t)n + 1);
    vsnprintf(small, (size_t)n + 1, fmt, ap);
    error_reset();
    if (!module && java_depth) {
        for (int i = 0; i < java_depth && i < 64; i++)
            buf_printf(&error_message, "Attempted to apply the operator overridden by the Java method\n%s,\n"
                       "but it produced the following error:\n", java_frames[i]);
        buf_puts(&error_message, small);
        loc_line(&error_message, e);
        e = NULL;
        module = 2;
    } else {
        buf_puts(&error_message, small);
    }
    free(small);
    java_depth = 0;   /* errors are caught outside every standard operator */
    error_expr = e;
    error_module = module;
    error_status = status;
    raise_error();
}

static _Noreturn void fail_at(Expr *e, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfail(e, 0, TPP_ERROR, fmt, ap);
}

/* An error TLC raises as a TLCRuntimeException, which exits with status. */
static _Noreturn void fail_runtime(Expr *e, int status, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfail(e, 2, status, fmt, ap);
}

/* An error in an operator of a standard module, which TLC implements in Java. */
static _Noreturn void fail_module(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfail(NULL, 1, TPP_ERROR, fmt, ap);
}

#define fail(...) fail_at(NULL, __VA_ARGS__)

/* A worker gives a state back to the search, which expands it itself. */
static _Noreturn void worker_cannot(void) { fail("A worker cannot expand this state."); }

/* ---------------------------------------------------------------- values */

/* Kinds, in the order values of different kinds sort (§7.4). */
enum {
    V_BOOL, V_INT, V_STR, V_MODEL, V_TUPLE, V_FCN, V_SET, V_INTERVAL,
    V_LAZY,     /* a set kept as its definition: see LazyKind */
    V_LAMBDA,   /* a function over an infinite domain, kept as its definition */
    V_OPER      /* an operator passed as an argument; never a TLA+ value */
};

/* How a function was made, which decides TLC's errors applying it: a
   record, or [x \in S |-> e] not yet stored in a state or a constant. */
/* MADE_LAMBDA: computed by [x \in S |-> e];
   MADE_HELD: a V_SET that TLC keeps as its definition, a UNION or a \cup, \cap
   or \ of such sets, so that a filter or function over it is lazy (§6.5) */
enum Made { MADE_PLAIN, MADE_RECORD, MADE_LAMBDA, MADE_HELD };

enum LazyKind {
    L_NAT, L_INT, L_STRING, L_SUBSET, L_FCNSET, L_RCDSET, L_SEQ, L_FILTER, L_TIMES, L_ANY,
    L_UNION,    /* UNION S that TLC keeps as it is, S not a set of sets of their elements: S in a, its
                   elements once enumerated in b, and in set once TLC keeps them too (union_cache);
                   n while a is to be sorted (union_sets) */
    L_CUP, L_CAP, L_DIFF
};

struct Value {
    unsigned char kind;
    union {
        unsigned char lazy;  /* LazyKind of a V_LAZY */
        unsigned char made;  /* Made of a V_FCN or V_SET */
        unsigned char keep;  /* V_LAMBDA: TLC's KeepLazy when it was made, which it is computed in full with */
    };
    unsigned char perm;      /* a V_LAZY made in the permanent arena */
    int n;                   /* elements of a set, tuple, or function */
    union {
        int64_t i;           /* V_BOOL, V_INT, V_STR, V_MODEL (string id) */
        struct { Value **elems; uint64_t hash; Expr *source; Value *product; } set;   /* V_SET, sorted, distinct */
        /* product: the s1 \X ... \X sn TLC keeps, held enumerated */
        struct { Value **keys, **vals; uint64_t hash; Expr *source; } fcn;
        /* V_TUPLE has no keys; source: the <<...>>, [...] or {...} that
           made it, which TLC names when applying or fingerprinting it
           fails, until the state holding it is stored */
        /* hash: the sum of the elements' or pairs' hashes, once known; else 0 */
        struct { int64_t lo, hi; } range;            /* V_INTERVAL */
        struct { Value *a, *b; union { Value **names; Value *set; }; Expr *expr; Env *env; Inst *inst; Def *binder; Value ***made_in; } l;
        /* set: an L_CUP, L_CAP or L_DIFF's elements, once enumerated */
        /* V_LAMBDA: [x \in a |-> expr] in env, with b for @, then n EXCEPT pairs (key, value);
           made_in: in a simulation, the current and next state it was made in */
        struct { Def *def; Env *env; Inst *inst; Expr *lambda; } op;
    } u;
};

static Value value_true = {.kind = V_BOOL, .u.i = 1};
static Value value_false = {.kind = V_BOOL, .u.i = 0};
#define TRUEV (&value_true)
#define FALSEV (&value_false)
#define BOOLV(b) ((b) ? TRUEV : FALSEV)

static Value *small_ints[1024];

static Value *mk_int(int64_t i) {
    if (i >= -1 && i < 1023 && small_ints[i + 1]) return small_ints[i + 1];
    Value *v = NEW(Value);
    v->kind = V_INT;
    v->u.i = i;
    return v;
}

/* One value for each string, and each model value, outside the arenas. */
static Value *atom_of(Value **at, int kind, int id) {
    if (!*at) {
        *at = xmalloc(sizeof(Value));
        **at = (Value){.kind = (unsigned char)kind, .u.i = id};
    }
    return *at;
}

static Value *mk_str(int id) { return atom_of(&strs[id].value, V_STR, id); }
static Value *mk_model(int id) { return atom_of(&strs[id].model, V_MODEL, id); }

static Value *mk_tuple(int n) {
    Value *v = NEW(Value);
    v->kind = V_TUPLE;
    v->n = n;
    v->u.fcn.vals = alloc_raw(sizeof(Value *) * (size_t)n);
    return v;
}

/* The characters of a string, which TLC's Sequences operators read as a sequence. */
static Value *str_chars(Value *s) {
    int n = strs[s->u.i].len;
    Value *t = mk_tuple(n);
    for (int i = 0; i < n; i++) t->u.fcn.vals[i] = mk_str(intern_n(strs[s->u.i].s + i, 1));
    return t;
}

static Value *substr(Value *s, int from, int len) { return mk_str(intern_n(strs[s->u.i].s + from, len)); }

static Value *mk_fcn(int n) {
    Value *v = NEW(Value);
    v->kind = V_FCN;
    v->n = n;
    v->u.fcn.keys = alloc_raw(sizeof(Value *) * (size_t)n);
    v->u.fcn.vals = alloc_raw(sizeof(Value *) * (size_t)n);
    return v;
}

static Value *mk_interval(int64_t lo, int64_t hi) {
    Value *v = NEW(Value);
    v->kind = V_INTERVAL;
    v->u.range.lo = lo;
    v->u.range.hi = hi;
    v->n = hi < lo ? 0 : hi - lo + 1 > INT32_MAX ? -1 : (int)(hi - lo + 1);
    return v;
}

/* The number of elements of an interval, which TLC keeps in 32 bits. */
static int interval_size(Value *v) {
    if (v->n < 0)
        fail("Size of interval value exceeds the maximum representable size (32bits): %lld..%lld.",
             (long long)v->u.range.lo, (long long)v->u.range.hi);
    return v->n;
}

static Value *mk_lazy(int kind) {
    Value *v = NEW(Value);
    v->kind = V_LAZY;
    v->lazy = (unsigned char)kind;
    v->perm = arena == &perm_arena;
    return v;
}

/* A value or binding made in the permanent arena computes what it keeps
   there, as TLC keeps it, but in the search: the arena to go back to. */
static Arena *keeping(int perm) {
    Arena *saved = arena;
    if (perm && arena != &scratch_arena) arena = &perm_arena;
    return saved;
}

/* Whether lazy s may keep a value made now: not one that dies before it. */
static int may_keep(Value *s) {
    if (s->perm && arena != &perm_arena) return 0;
    held_by(s);
    return 1;
}

static const char *kind_name(Value *v) {
    switch (v->kind) {
    case V_BOOL: return "boolean";
    case V_INT: return "integer";
    case V_STR: return "string";
    case V_MODEL: return "model value";
    case V_TUPLE: case V_FCN: return "function";
    default: return "set";
    }
}

static int is_set(Value *v) { return v->kind == V_SET || v->kind == V_INTERVAL || v->kind == V_LAZY; }

/* Nat, Int, STRING, Seq(S) and Any: what TLC calls overridden values. */
static int user_set(Value *s) {
    return s->kind == V_LAZY && (s->lazy == L_NAT || s->lazy == L_INT || s->lazy == L_STRING || s->lazy == L_SEQ || s->lazy == L_ANY);
}
/* How TLC names the kind of a value in error messages. */
static const char *kind_string(Value *v) {
    switch (v->kind) {
    case V_BOOL: return "a Boolean value";
    case V_INT: return "an integer";
    case V_STR: return "a string";
    case V_MODEL: return "a model value";
    case V_TUPLE: return "a tuple";
    case V_FCN: return v->made == MADE_RECORD ? "a record" : "a function  of the form (d1 :> e1 @@ ... @@ dN :> eN)";
    case V_SET: return "a set of the form {e1, ... ,eN}";
    case V_INTERVAL: return "a set of the form i..j";
    case V_OPER: return "an operator";
    case V_LAMBDA: return "a function of the form  [x \\in S |-> expr]";
    default: break;
    }
    switch (v->lazy) {
    case L_SUBSET: return "a set of the form SUBSET S";
    case L_FCNSET: return "a set of the form [S -> T]";
    case L_RCDSET: return "a set of the form [d1 : S1, ... , dN : SN]";
    case L_FILTER: return "a set of the form {x \\in S : expr}";
    case L_TIMES: return "a cartesian product";
    case L_CUP: return "a set of the form S \\cup T";
    case L_CAP: return "a set of the form S \\cap T";
    case L_DIFF: return "a set of the form S \\ T";
    default: return "a special set constant";
    }
}

static int is_fcn(Value *v) { return v->kind == V_TUPLE || v->kind == V_FCN; }

/* ---------------------------------------------------------------- printing */

static void print_value(Buf *b, Value *v);
static Value *realize_lambda(Value *f);
static void realize_both(Value **a, Value **b);
/* A lazy function over a finite domain as the function it is (§6.6). */
static inline Value *realize(Value *v) { return v->kind == V_LAMBDA ? realize_lambda(v) : v; }
static Value *try_realize(Value *f);
static void print_lambda(Buf *b, Value *f);
static void print_filter(Buf *b, Value *s);
static Value *normalize_set(Value *v);
static __attribute__((noinline)) int lazy_is_infinite(Value *s);
static inline int set_is_infinite(Value *s) { return s->kind == V_LAZY && lazy_is_infinite(s); }
static int64_t set_size(Value *s);
static Value *enumerate_other(Value *s);
static inline Value *enumerate(Value *s) { return s->kind == V_SET ? s : enumerate_other(s); }
static Value *union_cache(Value *s);
static Value *union_sets(Value *s);
static int kinds_differ(Value *a, Value *b);
static int enumerable(Value *s);
static Value *select_members(Value *s, Value *t, int in);

/* Whether TLC enumerates S \cup T, S \cap T or S \ T it keeps as its
   definition, printing it. */
static int set_op_enumerable(Value *v) {
    Value *a = v->u.l.a, *b = v->u.l.b;
    if (v->lazy == L_CUP) return enumerable(a) && enumerable(b) && !kinds_differ(enumerate(a), enumerate(b));
    return enumerable(a) || (v->lazy == L_CAP && enumerable(b));
}

/* A lazy set TLC prints by its elements; others it prints as written. */
static int expandable(Value *v) {
    if (set_is_infinite(v)) return 0;
    int64_t n = set_size(v);
    return n >= 0 && n < 2000000;
}

/* A function whose domain is 1..n prints as a tuple. */
static int fcn_is_tuple(Value *f) {
    if (f->kind == V_TUPLE) return 1;
    for (int i = 0; i < f->n; i++)
        if (f->u.fcn.keys[i]->kind != V_INT || f->u.fcn.keys[i]->u.i != i + 1) return 0;
    return 1;
}

static int fcn_is_record(Value *f) {
    if (f->kind != V_FCN || f->n == 0) return 0;
    for (int i = 0; i < f->n; i++)
        if (f->u.fcn.keys[i]->kind != V_STR) return 0;
    return 1;
}

/* TLC prints a function as a record if each key is a name: letters,
   digits and _, a letter among them, and no WF_ or SF_ of more. */
static int printed_as_record(Value *f) {
    if (!fcn_is_record(f)) return 0;
    for (int i = 0; i < f->n; i++) {
        const char *k = S(f->u.fcn.keys[i]->u.i);
        int letter = 0;
        for (const char *p = k; *p; p++) {
            if (*p != '_' && !isalnum((unsigned char)*p)) return 0;
            letter |= isalpha((unsigned char)*p) != 0;
        }
        if (!letter || (strlen(k) >= 4 && (!strncmp(k, "WF_", 3) || !strncmp(k, "SF_", 3)))) return 0;
    }
    return 1;
}

static void print_string(Buf *b, int id) {
    buf_puts(b, "\"");
    for (const char *s = S(id); *s; s++) {
        switch (*s) {
        case '"': buf_puts(b, "\\\""); break;
        case '\\': buf_puts(b, "\\\\"); break;
        case '\n': buf_puts(b, "\\n"); break;
        case '\t': buf_puts(b, "\\t"); break;
        case '\r': buf_puts(b, "\\r"); break;
        case '\f': buf_puts(b, "\\f"); break;
        default: buf_add(b, s, 1);
        }
    }
    buf_puts(b, "\"");
}

static void print_value(Buf *b, Value *v) {
    if (v->kind == V_LAMBDA) v = try_realize(v);
    if (v->kind == V_LAMBDA) { print_lambda(b, v); return; }
    switch (v->kind) {
    case V_BOOL: buf_puts(b, v->u.i ? "TRUE" : "FALSE"); return;
    case V_INT: buf_printf(b, "%lld", (long long)v->u.i); return;
    case V_STR: print_string(b, (int)v->u.i); return;
    case V_MODEL: buf_puts(b, S(v->u.i)); return;
    case V_INTERVAL:
        if (v->n == 0) buf_puts(b, "{}");
        else buf_printf(b, "%lld..%lld", (long long)v->u.range.lo, (long long)v->u.range.hi);
        return;
    case V_LAZY:
        if (v->lazy == L_NAT) { buf_puts(b, "Nat"); return; }
        if (v->lazy == L_INT) { buf_puts(b, "Int"); return; }
        if (v->lazy == L_STRING) { buf_puts(b, "STRING"); return; }
        if (v->lazy == L_ANY) { buf_puts(b, "ANY"); return; }
        if (v->lazy == L_SEQ) { buf_puts(b, "Seq("); print_value(b, v->u.l.a); buf_puts(b, ")"); return; }
        if (v->lazy == L_UNION) {
            int infinite = 0;
            for (int i = 0; i < v->u.l.a->n; i++) infinite |= is_set(v->u.l.a->u.set.elems[i]) && set_is_infinite(v->u.l.a->u.set.elems[i]);
            if (infinite) { buf_puts(b, "UNION "); print_value(b, v->u.l.a); return; }
        }
        if (v->lazy >= L_CUP && !set_op_enumerable(v)) {
            print_value(b, v->u.l.a);
            buf_puts(b, v->lazy == L_CUP ? " \\cup " : v->lazy == L_CAP ? " \\cap " : " \\ ");
            print_value(b, v->u.l.b);
            return;
        }
        if (expandable(v)) { print_value(b, normalize_set(v)); return; }
        if (v->lazy == L_SUBSET) { buf_puts(b, "SUBSET "); print_value(b, v->u.l.a); return; }
        if (v->lazy == L_FCNSET) {
            buf_puts(b, "[");
            print_value(b, v->u.l.a);
            buf_puts(b, " -> ");
            print_value(b, v->u.l.b);
            buf_puts(b, "]");
            return;
        }
        if (v->lazy == L_RCDSET) {
            buf_puts(b, "[");
            for (int i = 0; i < v->n; i++) {
                buf_printf(b, "%s%s: ", i ? ", " : "", S(v->u.l.names[i]->u.i));
                print_value(b, v->u.l.names[v->n + i]);
            }
            buf_puts(b, "]");
            return;
        }
        if (v->lazy == L_TIMES) {
            buf_puts(b, "(");
            for (int i = 0; i < v->n; i++) {
                if (i) buf_puts(b, " \\X ");
                print_value(b, v->u.l.names[v->n + i]);
            }
            buf_puts(b, ")");
            return;
        }
        if (v->lazy == L_FILTER && set_is_infinite(v->u.l.a)) { print_filter(b, v); return; }
        print_value(b, normalize_set(v));
        return;
    case V_SET:
        buf_puts(b, "{");
        for (int i = 0; i < v->n; i++) {
            if (i) buf_puts(b, ", ");
            print_value(b, v->u.set.elems[i]);
        }
        buf_puts(b, "}");
        return;
    case V_TUPLE: case V_FCN:
        if (v->n == 0) { buf_puts(b, "<<>>"); return; }
        if (fcn_is_tuple(v)) {
            buf_puts(b, "<<");
            for (int i = 0; i < v->n; i++) {
                if (i) buf_puts(b, ", ");
                print_value(b, v->u.fcn.vals[i]);
            }
            buf_puts(b, ">>");
        } else if (printed_as_record(v)) {
            buf_puts(b, "[");
            for (int i = 0; i < v->n; i++) {
                if (i) buf_puts(b, ", ");
                buf_printf(b, "%s |-> ", S(v->u.fcn.keys[i]->u.i));
                print_value(b, v->u.fcn.vals[i]);
            }
            buf_puts(b, "]");
        } else {
            buf_puts(b, "(");
            for (int i = 0; i < v->n; i++) {
                if (i) buf_puts(b, " @@ ");
                print_value(b, v->u.fcn.keys[i]);
                buf_puts(b, " :> ");
                print_value(b, v->u.fcn.vals[i]);
            }
            buf_puts(b, ")");
        }
        return;
    case V_OPER: buf_puts(b, "<operator>"); return;
    }
}

static char *value_str(Value *v) {
    Buf b = {0};
    print_value(&b, v);
    return buf_take(&b);
}

/* TLC's pretty printer (tlc2.pprint.Format) breaks a value longer than
   the line width, element by element (§7.6). */

/* Whether TLC's parser fails on a name it prints bare, a model value or
   a record's field: it takes only letters and digits. */
static int pp_odd_name(int name) {
    for (const char *p = S(name); *p; p++) if (!isalnum((unsigned char)*p)) return 1;
    return 0;
}

static int pp_plain(Value *v) {
    if (v->kind == V_LAMBDA) return try_realize(v)->kind == V_LAMBDA;   /* TLC cannot parse <expression ...> */
    if (v->kind == V_MODEL) return pp_odd_name((int)v->u.i);
    if (v->kind == V_INTERVAL || (v->kind == V_LAZY && !expandable(v))) return 1;
    if (v->kind == V_LAZY) return pp_plain(normalize_set(v));
    if (v->kind == V_SET)
        for (int i = 0; i < v->n; i++) if (pp_plain(v->u.set.elems[i])) return 1;
    if (is_fcn(v)) {
        int record = v->kind == V_FCN && printed_as_record(v);
        for (int i = 0; i < v->n; i++)
            if (pp_plain(v->u.fcn.vals[i]) || (v->kind == V_FCN && (record ? pp_odd_name((int)v->u.fcn.keys[i]->u.i) : pp_plain(v->u.fcn.keys[i])))) return 1;
    }
    return 0;
}

static void pp_format(Buf *b, Value *v, int width, int trailer, const char *pad);

static void pp_values(Buf *b, Value **vals, int n, int width, int trailer, const char *pad) {
    for (int i = 0; i < n; i++) {
        if (i + 1 < n) {
            pp_format(b, vals[i], width, 1, pad);
            buf_puts(b, ",\n");
            buf_puts(b, pad);
        } else {
            pp_format(b, vals[i], width, trailer, pad);
        }
    }
}

static char *pad_more(const char *pad, int n) {
    size_t len = strlen(pad);
    char *s = alloc(len + (size_t)n + 1);
    memcpy(s, pad, len);
    memset(s + len, ' ', (size_t)n);
    return s;
}

static void pp_pairs(Buf *b, Value *f, int record, int width, int trailer, const char *pad) {
    const char *sep = record ? "," : " @@";
    const char *div = record ? " |->" : " :>";
    int seplen = (int)strlen(sep), divlen = (int)strlen(div), divpad = 4;
    char *inner = pad_more(pad, divpad);
    for (int i = 0; i < f->n; i++) {
        Value *key = f->u.fcn.keys[i], *val = f->u.fcn.vals[i];
        char *ks = record ? S(key->u.i) : value_str(key);
        char *vs = value_str(val);
        int kl = (int)strlen(ks), vl = (int)strlen(vs);
        int last = i + 1 == f->n;
        int tail = last ? trailer : 0;
        if (kl + divlen + 1 + vl + (last ? 0 : seplen) <= width - tail) {
            buf_printf(b, "%s%s %s", ks, div, vs);
        } else {
            if (kl + divlen <= width || record) buf_printf(b, "%s%s", ks, div);
            else { pp_format(b, key, width, trailer + divlen, pad); buf_puts(b, div); }
            buf_printf(b, "\n%s    ", pad);
            pp_format(b, val, width - divpad, last ? trailer : seplen, inner);
        }
        if (!last) { buf_printf(b, "%s\n%s", sep, pad); }
    }
}

static void pp_format(Buf *b, Value *v, int width, int trailer, const char *pad) {
    if (v->kind == V_LAMBDA) v = try_realize(v);
    if (v->kind == V_LAZY && expandable(v)) v = normalize_set(v);
    char *flat = value_str(v);
    if ((int)strlen(flat) <= width - trailer || pp_plain(v) || v->n == 0) { buf_puts(b, flat); return; }
    if (v->kind == V_SET) {
        buf_puts(b, "{ ");
        pp_values(b, v->u.set.elems, v->n, width - 2, trailer + 2, pad_more(pad, 2));
        buf_puts(b, " }");
    } else if (is_fcn(v) && fcn_is_tuple(v)) {
        buf_puts(b, "<< ");
        pp_values(b, v->u.fcn.vals, v->n, width - 3, trailer + 3, pad_more(pad, 3));
        buf_puts(b, " >>");
    } else if (is_fcn(v)) {
        int record = printed_as_record(v);
        buf_puts(b, record ? "[ " : "( ");
        pp_pairs(b, v, record, width - 2, trailer + 2, pad_more(pad, 2));
        buf_puts(b, record ? " ]" : " )");
    } else {
        buf_puts(b, flat);
    }
}

static char *pretty(Value *v) {
    Buf b = {0};
    pp_format(&b, v, 80, 0, "");
    return buf_take(&b);
}

/* ---------------------------------------------------------------- environments */

/* A context is where an expression is evaluated: the bindings of bound
   names and operator parameters, and the module instance (§4.3). */
typedef struct { Env *env; Inst *inst; } Ctx;

/* A binding. An operator's argument is bound unevaluated, as `e` in its
   caller's context, and evaluated where it is used (§5.2). */
struct Env {
    Def *def;
    Value *v;
    Expr *e;
    Env *ctx_env;
    Inst *ctx_inst;
    Env *next;
    unsigned char perm;   /* allocated in the permanent arena */
    unsigned char stateful;   /* v was read from the current state */
};

/* An instance: the module instance definition and the context of the
   instantiation, where the substitutions are evaluated. */
struct Inst {
    Def *def;
    Env *env;
    Inst *parent;
};

typedef Value *(*EvalFn)(Expr *e, Ctx c);
static inline Value *eval(Expr *e, Ctx c);
static int eval_bool(Expr *e, Ctx c);
static Value *filter_member_test(Value *s, Value *v);

/* ---------------------------------------------------------------- comparison */

static int vcmp(Value *a, Value *b);
static int veq(Value *a, Value *b);

static Value *fcn_key(Value *f, int i) { return f->kind == V_TUPLE ? mk_int(i + 1) : f->u.fcn.keys[i]; }

/* The expression that made a value, as TLC keeps it to name in a
   fingerprint stack trace when an error arises in one of its methods. */
static Expr *value_source(Value *v) {
    return v->kind == V_SET ? v->u.set.source : v->kind == V_TUPLE || v->kind == V_FCN ? v->u.fcn.source : NULL;
}

static _Noreturn void compare_error(Value *a, Value *b) {
    fp_enter(value_source(a));
    const char *k = kind_name(a);
    if (a->kind == V_FCN && a->made == MADE_RECORD)
        fail("Attempted to compare record:\n%s\nwith non-record\n%s", pretty(a), pretty(b));
    if (is_fcn(a)) fail("Attempted to compare the function %s with the value:\n%s", pretty(a), pretty(b));
    if (is_set(a)) fail("Attempted to compare the set %s with the value:\n%s", pretty(a), pretty(b));
    fail("Attempted to compare %s %s with non-%s:\n%s", k, pretty(a), k, pretty(b));
}

/* Nat or Int, which TLC orders and compares without enumerating: Nat first. */
static int number_set(Value *v) { return v->kind == V_LAZY && v->lazy <= L_INT; }

/* TLC's error comparing sets of which one is infinite. */
static _Noreturn void incomparable(Value *a, Value *b) {
    /* Two overridden values compare by the first's module. */
    if (user_set(a) && user_set(b)) fail_module("Attempted to compare %s with the value\n%s", pretty(a), pretty(b));
    fp_enter(value_source(a));
    if (user_set(a)) fail("Attempted to compare overridden value %s with non-overridden value:\n%s", pretty(a), pretty(b));
    if (user_set(b)) fail("Attempted to check equality of the set %s with the value:\n%s", pretty(enumerate(a)), pretty(b));
    enumerate(set_is_infinite(a) ? a : b);   /* fails with TLC's error for that set */
    abort();
}

/* The standard order (§7.4). Model values come first; other values of
   different kinds cannot be compared. */
static int vcmp(Value *a, Value *b) {
    if (a->kind == b->kind && a->kind <= V_MODEL) return (a->u.i > b->u.i) - (a->u.i < b->u.i);
    if (a == b) return 0;
    if (a->kind == V_LAMBDA || b->kind == V_LAMBDA) realize_both(&a, &b);
    /* Comparing the parts of a value TLC made with a source names it in
       the fingerprint stack trace of an error there. */
    Expr *src = value_source(a);
    int c = 0;
    if (a->kind == V_TUPLE && b->kind == V_TUPLE) {
        if (a->n != b->n) return a->n < b->n ? -1 : 1;
        fp_enter(src);
        for (int i = 0; i < a->n && !c; i++) {
            Value *x = a->u.fcn.vals[i], *y = b->u.fcn.vals[i];
            c = x->kind == y->kind && x->kind <= V_MODEL ? (x->u.i > y->u.i) - (x->u.i < y->u.i) : vcmp(x, y);
        }
        fp_leave(src);
        return c;
    }
    if (number_set(a) && number_set(b)) return (a->lazy > b->lazy) - (a->lazy < b->lazy);
    if (a->kind == V_MODEL || b->kind == V_MODEL) {
        if (a->kind != b->kind) return a->kind == V_MODEL ? -1 : 1;
        return a->u.i < b->u.i ? -1 : a->u.i > b->u.i;
    }
    if (is_fcn(a) && is_fcn(b)) {
        if (a->n != b->n) return a->n < b->n ? -1 : 1;
        fp_enter(src);
        for (int i = 0; i < a->n && !c; i++) {
            if (a->kind != V_TUPLE || b->kind != V_TUPLE) c = vcmp(fcn_key(a, i), fcn_key(b, i));
            if (!c) c = vcmp(a->u.fcn.vals[i], b->u.fcn.vals[i]);
        }
        fp_leave(src);
        return c;
    }
    if (is_set(a) && is_set(b)) {
        if (a->kind == V_LAZY && b->kind == V_LAZY && a->lazy == b->lazy && a->lazy <= L_STRING) return 0;
        if (a->kind == V_SET && user_set(b)) {   /* TLC's SetEnumValue cannot enumerate the other */
            fp_enter(src);
            fail("Attempted to compare the set %s with the value:\n%s", pretty(a), pretty(b));
        }
        if (set_is_infinite(a) || set_is_infinite(b)) incomparable(user_set(b) ? b : a, user_set(b) ? a : b);   /* as TLC sorts */
        a = enumerate(union_cache(a));
        b = enumerate(b);
        if (a->n != b->n) return a->n < b->n ? -1 : 1;
        fp_enter(src);
        for (int i = 0; i < a->n && !c; i++) {
            Value *x = a->u.set.elems[i], *y = b->u.set.elems[i];
            c = x->kind == y->kind && x->kind <= V_MODEL ? (x->u.i > y->u.i) - (x->u.i < y->u.i) : vcmp(x, y);
        }
        fp_leave(src);
        return c;
    }
    if (a->kind != b->kind || a->kind == V_OPER) compare_error(a, b);
    return a->u.i < b->u.i ? -1 : a->u.i > b->u.i;
}

/* vcmp, inline for atoms and tuples of atoms, the common elements of sets. */
static inline int vcmp_quick(Value *a, Value *b) {
    if (a->kind == b->kind && a->kind <= V_MODEL) return (a->u.i > b->u.i) - (a->u.i < b->u.i);
    if (a->kind == V_TUPLE && b->kind == V_TUPLE && a->n == b->n) {
        for (int i = 0; i < a->n; i++) {
            Value *x = a->u.fcn.vals[i], *y = b->u.fcn.vals[i];
            if (x->kind != y->kind || x->kind > V_MODEL) return vcmp(a, b);
            if (x->u.i != y->u.i) return x->u.i < y->u.i ? -1 : 1;
        }
        return 0;
    }
    return vcmp(a, b);
}

static _Noreturn void equality_error(Value *a, Value *b) {
    fp_enter(value_source(a));
    const char *k = kind_name(a);
    if (a->kind == V_BOOL) fail("Attempted to compare equality of boolean %s with non-boolean:\n%s", pretty(a), pretty(b));
    if (a->kind == V_FCN && a->made == MADE_RECORD)
        fail("Attempted to check equality of record:\n%s\nwith non-record\n%s", pretty(a), pretty(b));
    if (user_set(a)) fail("Attempted to compare overridden value %s with non-overridden value:\n%s", pretty(a), pretty(b));
    if (is_fcn(a)) fail("Attempted to check equality of the function %s with the value:\n%s", pretty(a), pretty(b));
    if (is_set(a)) fail("Attempted to check equality of the set %s with the value:\n%s", pretty(a), pretty(b));
    fail("Attempted to check equality of %s %s with non-%s:\n%s", k, pretty(a), k, pretty(b));
}

/* Whether TLC makes a record of the function, its domain all strings. */
static int as_record(Value *f) {
    if (f->kind == V_TUPLE) return f->n == 0;
    for (int i = 0; i < f->n; i++) if (f->u.fcn.keys[i]->kind != V_STR) return 0;
    return 1;
}
static int veq(Value *a, Value *b) {
    if (a == b) return 1;
    if (a->kind == V_TUPLE && b->kind == V_STR) return veq(a, str_chars(b));   /* TLC's tuple takes a string as its characters */
    if (a->kind == V_LAMBDA || b->kind == V_LAMBDA) realize_both(&a, &b);
    if (a->kind == V_MODEL || b->kind == V_MODEL) return a->kind == b->kind && a->u.i == b->u.i;
    Expr *src = value_source(a);   /* named in an error in its parts, as vcmp */
    int same = 1;
    if (is_fcn(a) && is_fcn(b)) {
        if (a->made == MADE_RECORD && !as_record(b)) equality_error(a, b);
        if (a->n != b->n) return 0;
        fp_enter(src);
        for (int i = 0; i < a->n && same; i++)
            same = ((a->kind == V_TUPLE && b->kind == V_TUPLE) || veq(fcn_key(a, i), fcn_key(b, i)))
                && veq(a->u.fcn.vals[i], b->u.fcn.vals[i]);
        fp_leave(src);
        return same;
    }
    if (is_set(a) && is_set(b)) {
        if (a->kind == V_INTERVAL && b->kind == V_INTERVAL)
            return interval_size(a) == interval_size(b) && (a->n == 0 || a->u.range.lo == b->u.range.lo);
        if (a->kind == V_LAZY && b->kind == V_LAZY && a->lazy == b->lazy && a->lazy <= L_STRING) return 1;
        if (number_set(a) && number_set(b)) return 0;
        if (set_is_infinite(a) || set_is_infinite(b)) {
            if (a->kind == V_LAZY && b->kind == V_LAZY && a->lazy == b->lazy && a->lazy == L_SEQ)
                return veq(a->u.l.a, b->u.l.a);
            incomparable(a, b);
        }
        a = enumerate(union_cache(a));
        b = enumerate(b);
        if (a->n != b->n) return 0;
        fp_enter(src);
        for (int i = 0; i < a->n && same; i++) same = veq(a->u.set.elems[i], b->u.set.elems[i]);
        fp_leave(src);
        return same;
    }
    if (a->kind != b->kind || a->kind == V_OPER) equality_error(a, b);
    return a->u.i == b->u.i;
}

/* ---------------------------------------------------------------- sets */

static Value *mk_set(int n) {
    Value *v = NEW(Value);
    v->kind = V_SET;
    v->n = n;
    v->u.set.elems = alloc_raw(sizeof(Value *) * (size_t)n);
    return v;
}

static int cmp_for_sort(const void *x, const void *y) { return vcmp(*(Value **)x, *(Value **)y); }

/* Sorts the elements and removes duplicates. Small sets, often built in
   order, are sorted by insertion, which finds any duplicate next to where
   it stops. */
static Value *set_normalize(Value *s) {
    Value **e = s->u.set.elems;
    int n = s->n, dup = 0;
    if (n < 2) return s;
    if (n <= 32) {
        for (int i = 1; i < n; i++) {
            Value *x = e[i];
            int j = i, c = 1;
            while (j > 0 && (c = vcmp(x, e[j - 1])) < 0) { e[j] = e[j - 1]; j--; }   /* as TLC: the new one first */
            e[j] = x;
            dup |= c == 0;
        }
    } else {
        qsort(e, (size_t)n, sizeof(Value *), cmp_for_sort);
        dup = 1;
    }
    if (dup) {
        int k = 1;
        for (int i = 1; i < n; i++) if (vcmp(e[k - 1], e[i]) != 0) e[k++] = e[i];
        s->n = k;
    }
    return s;
}

/* A list that builds a set. */
typedef struct { Value **v; int n, cap; } List;

static void list_add(List *l, Value *v) {
    if (l->n == l->cap) {
        int cap = l->cap ? l->cap * 2 : 8;
        Value **grown = alloc_raw(sizeof(Value *) * (size_t)cap);
        if (l->n) memcpy(grown, l->v, sizeof(Value *) * (size_t)l->n);
        l->v = grown;
        l->cap = cap;
    }
    l->v[l->n++] = v;
}

/* The enumerated S of {x \in S : p(x)}, whose elements TLC filters as
   it takes them. */
static Value *filter_base(Value *s) {
    if (!is_set(s->u.l.a) || user_set(s->u.l.a))
        fail("Attempted to enumerate { x \\in S : p(x) } when S:\n%s\nis not enumerable", pretty(s->u.l.a));
    return enumerate(s->u.l.a);
}

static Value *list_set(List *l) {
    Value *s = mk_set(l->n);
    if (l->n) memcpy(s->u.set.elems, l->v, sizeof(Value *) * (size_t)l->n);
    return set_normalize(s);
}

static int lazy_is_infinite(Value *s) {
    switch (s->lazy) {
    case L_NAT: case L_INT: case L_STRING: case L_SEQ: case L_ANY: return 1;
    case L_SUBSET: case L_FILTER: case L_UNION: return set_is_infinite(s->u.l.a);
    case L_FCNSET: case L_CUP: return set_is_infinite(s->u.l.a) || set_is_infinite(s->u.l.b);
    case L_CAP: return set_is_infinite(s->u.l.a) && set_is_infinite(s->u.l.b);
    case L_DIFF: return set_is_infinite(s->u.l.a);
    case L_RCDSET: case L_TIMES:
        for (int i = 0; i < s->n; i++) if (set_is_infinite(s->u.l.names[s->n + i])) return 1;
        return 0;
    }
    return 0;
}

static Value *mk_fcn_from(List *keys, List *vals);

/* All functions from the elements of `dom` (sorted) to elements of `rng`,
   made in the standard order: the last value varies fastest. */
static Value *enumerate_functions(Value **dom, int nd, Value *rng) {
    rng = enumerate(rng);
    int64_t total = 1;
    for (int i = 0; i < nd; i++) {
        total *= rng->n;
        if (total > 100000000) fail("Attempted to enumerate a set of functions with too many elements.");
    }
    Value *out = mk_set((int)total);
    Value **keys = alloc_raw(sizeof(Value *) * (size_t)nd);   /* the functions share them */
    if (nd) memcpy(keys, dom, sizeof(Value *) * (size_t)nd);
    int *idx = calloc((size_t)nd + 1, sizeof(int));
    for (int64_t k = 0; k < total; k++) {
        Value *f = out->u.set.elems[k] = NEW(Value);
        f->kind = V_FCN;
        f->n = nd;
        f->u.fcn.keys = keys;
        f->u.fcn.vals = alloc_raw(sizeof(Value *) * (size_t)nd);
        for (int i = 0; i < nd; i++) f->u.fcn.vals[i] = rng->u.set.elems[idx[i]];
        for (int i = nd - 1; i >= 0; i--) { if (++idx[i] < rng->n) break; idx[i] = 0; }
    }
    free(idx);
    return out;
}

/* The elements of a finite set, as a normalized V_SET. */
static _Noreturn void count_infinite(Value *s);

static Value *cup_merge(Value *a, Value *b);
static _Noreturn void sort_error(Value *a, Value *b);
static Value **elements_slot(Value *s);
static Value *enumerate_lazy(Value *s);

static Value *enumerate_other(Value *s) {
    if (s->kind == V_INTERVAL) {
        Value *r = mk_set(interval_size(s));
        for (int i = 0; i < s->n; i++) r->u.set.elems[i] = mk_int(s->u.range.lo + i);
        return r;
    }
    if (s->kind != V_LAZY) fail("Attempted to enumerate %s, which is not a set.", pretty(s));
    Value **slot = elements_slot(s);
    if (!slot) return enumerate_lazy(s);
    if (!*slot) {
        Arena *saved = arena;
        arena = &perm_arena;
        *slot = enumerate_lazy(s);
        arena = saved;
    }
    return *slot;
}

/* Where a lazy set made in the permanent arena keeps its elements, if it
   holds no context: they never change. */
static Value **elements_slot(Value *s) {
    if (!s->perm) return NULL;
    switch (s->lazy) {
    case L_SUBSET: case L_TIMES: case L_RCDSET: return &s->u.l.b;
    case L_FCNSET: return &s->u.l.set;
    default: return NULL;
    }
}

static Value *enumerate_lazy(Value *s) {
    switch (s->lazy) {
    /* A set TLC made with a source names it if enumerating it fails. */
    case L_SUBSET: {
        fp_enter(s->u.l.expr);
        if (set_is_infinite(s->u.l.a)) count_infinite(s->u.l.a);
        Value *base = enumerate(s->u.l.a);
        if (base->n > 24) fail("Attempted to enumerate SUBSET of a set with too many elements.");
        /* In the standard order, so nothing to sort: smaller subsets first,
           then those of each size in the order of their elements. */
        int n = base->n, at = 0, c[24];
        Value *out = mk_set(1 << n);
        for (int k = 0; k <= n; k++) {
            for (int i = 0; i < k; i++) c[i] = i;
            for (;;) {
                Value *sub = out->u.set.elems[at++] = mk_set(k);
                for (int i = 0; i < k; i++) sub->u.set.elems[i] = base->u.set.elems[c[i]];
                int i = k - 1;
                while (i >= 0 && c[i] == n - k + i) i--;
                if (i < 0) break;
                c[i]++;
                for (int j = i + 1; j < k; j++) c[j] = c[j - 1] + 1;
            }
        }
        fp_leave(s->u.l.expr);
        return out;
    }
    case L_FCNSET: {
        fp_enter(s->u.l.expr);
        for (int i = 0; i < 2; i++)
            if (set_is_infinite(i ? s->u.l.b : s->u.l.a))
                fail("Attempted to enumerate a set of the form [D -> R],but the %s:\n%s\ncannot be enumerated.",
                     i ? "range R" : "domain D", pretty(i ? s->u.l.b : s->u.l.a));
        Value *dom = enumerate(s->u.l.a);
        Value *out = enumerate_functions(dom->u.set.elems, dom->n, s->u.l.b);
        fp_leave(s->u.l.expr);
        return out;
    }
    case L_RCDSET: case L_TIMES: {
        fp_enter(s->u.l.expr);
        int n = s->n;
        Value **sets = NEWN(Value *, n);
        int64_t total = 1;
        for (int i = 0; i < n; i++) {
            Value *part = s->u.l.names[n + i];
            if (set_is_infinite(part) && s->lazy == L_TIMES)
                fail("Attempted to enumerate a set of the form s1 \\X s2 ... \\X sn,\nbut can't enumerate s%d:\n%s", i, pretty(part));
            if (set_is_infinite(part))
                fail("Attempted to enumerate a set of the form [l1 : v1, ..., ln : vn],\n"
                     "but can't enumerate the value of the `%s' field:\n%s", S(s->u.l.names[i]->u.i), pretty(part));
            sets[i] = enumerate(part);
            total *= sets[i]->n;
            if (total > 100000000) fail("Attempted to enumerate a set with too many elements.");
        }
        /* Made in the standard order, the last part varying fastest: the
           parts, and a record's fields, are sorted. */
        Value *out = mk_set((int)total);
        int *idx = calloc((size_t)n + 1, sizeof(int));
        for (int64_t k = 0; k < total; k++) {
            Value *t;
            if (s->lazy == L_TIMES) {
                t = mk_tuple(n);
                for (int i = 0; i < n; i++) t->u.fcn.vals[i] = sets[i]->u.set.elems[idx[i]];
            } else {
                t = mk_fcn(n);
                t->made = MADE_RECORD;
                for (int i = 0; i < n; i++) { t->u.fcn.keys[i] = s->u.l.names[i]; t->u.fcn.vals[i] = sets[i]->u.set.elems[idx[i]]; }
            }
            out->u.set.elems[k] = t;
            for (int i = n - 1; i >= 0; i--) { if (++idx[i] < sets[i]->n) break; idx[i] = 0; }
        }
        free(idx);
        fp_leave(s->u.l.expr);
        return out;
    }
    case L_FILTER: {
        if (s->u.l.b) return s->u.l.b;
        Arena *saved = keeping(s->perm);
        Value *base = filter_base(s);
        List out = {0};
        for (int i = 0; i < base->n; i++)
            if (filter_member_test(s, base->u.set.elems[i]) == TRUEV) list_add(&out, base->u.set.elems[i]);
        Value *set = list_set(&out);
        /* Keep the elements unless they would outlive the filter's arena. */
        if (may_keep(s)) s->u.l.b = set;
        arena = saved;
        return set;
    }
    case L_UNION: {
        /* As TLC: each element enumerated, unless it is a set it defines. */
        if (s->u.l.b) return s->u.l.b;
        Arena *saved = keeping(s->perm);
        union_sets(s);
        List l = {0};
        fp_enter(s->u.l.expr);
        for (int i = 0; i < s->u.l.a->n; i++) {
            Value *t = s->u.l.a->u.set.elems[i];
            if (user_set(t)) fail("Attempted to enumerate UNION(s), but some element of s is nonenumerable.");
            t = enumerate(t);
            for (int j = 0; j < t->n; j++) list_add(&l, t->u.set.elems[j]);
        }
        Value *set = list_set(&l);
        fp_leave(s->u.l.expr);
        if (may_keep(s)) s->u.l.b = set;
        arena = saved;
        return set;
    }
    case L_CUP: case L_CAP: case L_DIFF: {
        /* As TLC: S \cup T of both, S \cap T of an enumerable one, S \ T of S. */
        if (s->u.l.set) return s->u.l.set;
        Arena *saved = keeping(s->perm);
        Value *a = s->u.l.a, *b = s->u.l.b, *set;
        fp_enter(s->u.l.expr);
        if (s->lazy == L_CUP) {
            if (!enumerable(a) || !enumerable(b))
                fail("Attempted to enumerate S \\cup T when S:\n%s\nand T:\n%s\nare not both enumerable", pretty(a), pretty(b));
            if (kinds_differ(enumerate(a), enumerate(b))) sort_error(a, b);
            set = cup_merge(a, b);
        } else if (s->lazy == L_CAP) {
            if (enumerable(a)) set = select_members(a, b, 1);
            else if (enumerable(b)) set = select_members(b, a, 1);
            else fail("Attempted to enumerate S \\cap T when neither S:\n%s\nnor T:\n%s\nis enumerable", pretty(a), pretty(b));
        } else {
            if (!enumerable(a)) fail("Attempted to enumerate S \\ T when S:\n%s\nis not enumerable.", pretty(a));
            set = select_members(a, b, 0);
        }
        fp_leave(s->u.l.expr);
        if (may_keep(s)) s->u.l.set = set;
        arena = saved;
        return set;
    }
    default:
        fail("TLC encountered a non-enumerable quantifier bound\n%s.", pretty(s));
    }
}

static Value *normalize_set(Value *v) { return enumerate(v); }

/* The sets of a UNION TLC keeps, sorted once it must: comparing them, an
   error names the UNION and the set of them. */
static Value *union_sets(Value *s) {
    if (s->n) {   /* in place: in its own arena */
        Arena *saved = arena;
        if (s->perm) arena = &perm_arena;
        Expr *sets = s->u.l.a->u.set.source;
        fp_enter(s->u.l.expr);
        fp_enter(sets);
        held_by(s);
        s->u.l.a = set_normalize(s->u.l.a);
        fp_leave(sets);
        fp_leave(s->u.l.expr);
        s->n = 0;
        arena = saved;
    }
    return s->u.l.a;
}

/* As TLC's UnionValue once it compares, counts or fingerprints itself:
   it keeps its elements, which its quantifiers then take in order. */
static Value *union_cache(Value *s) {
    if (s->kind == V_LAZY && s->lazy == L_UNION && !s->u.l.set) {
        Arena *saved = keeping(s->perm);
        Value *e = enumerate(s);
        if (may_keep(s)) s->u.l.set = e;
        arena = saved;
    }
    return s;
}

/* The elements of s as TLC's quantifiers and actions take them: a UNION it
   keeps, those of each of its sets in turn, duplicates too; another set,
   in order. Only to iterate: the result is no normalized set. */
static Value *elements(Value *s) {
    if (s->kind != V_LAZY || s->lazy != L_UNION) return enumerate(s);
    if (s->u.l.set) return s->u.l.set;
    union_sets(s);
    List l = {0};
    fp_enter(s->u.l.expr);
    for (int i = 0; i < s->u.l.a->n; i++) {
        Value *t = s->u.l.a->u.set.elems[i];
        if (user_set(t)) fail("Attempted to enumerate UNION(s), but some element of s is nonenumerable.");
        t = elements(t);
        for (int j = 0; j < t->n; j++) list_add(&l, t->u.set.elems[j]);
    }
    fp_leave(s->u.l.expr);
    Value *r = mk_set(l.n);
    if (l.n) memcpy(r->u.set.elems, l.v, sizeof(Value *) * (size_t)l.n);
    return r;
}

/* TLC's error counting an infinite set: the first part it cannot count. */
static _Noreturn void count_infinite(Value *s) {
    for (Value *part = s; part; ) {
        s = part, part = NULL;
        if (user_set(s)) fail("Attempted to compute the number of elements in the overridden value %s.", pretty(s));
        if (s->lazy == L_SUBSET || s->lazy == L_FCNSET) part = set_is_infinite(s->u.l.a) ? s->u.l.a : s->u.l.b;
        if (s->lazy == L_RCDSET || s->lazy == L_TIMES)
            for (int i = 0; i < s->n && !part; i++) if (set_is_infinite(s->u.l.names[s->n + i])) part = s->u.l.names[s->n + i];
    }
    enumerate(s);   /* fails, as s is infinite */
    abort();
}

/* Lazy functions compared are computed, the left first; TLC cannot count
   an infinite domain. */
static void realize_both(Value **a, Value **b) {
    *a = realize(*a);
    if ((*a)->kind == V_LAMBDA) count_infinite((*a)->u.l.a);
    *b = realize(*b);
    if ((*b)->kind == V_LAMBDA) count_infinite((*b)->u.l.a);
}

/* The number of elements of a finite set, without enumerating a product of
   sets, or -1 when it is not a 32-bit int, TLC's count. */
static int64_t set_size(Value *s) {
    if (s->kind == V_INTERVAL) return interval_size(s);
    if (s->kind != V_LAZY || (s->lazy != L_SUBSET && s->lazy != L_FCNSET && s->lazy != L_RCDSET && s->lazy != L_TIMES))
        return enumerate(s)->n;
    int64_t n = 1, k;
    if (s->lazy == L_SUBSET) {
        if ((k = set_size(s->u.l.a)) < 0 || k > 30) return -1;
        return (int64_t)1 << k;
    }
    if (s->lazy == L_FCNSET) {
        int64_t r = set_size(s->u.l.b);
        if ((k = set_size(s->u.l.a)) < 0 || r < 0) return -1;
        for (; k > 0 && n; k--) if ((n *= r) > 2147483647) return -1;
        return n;
    }
    for (int i = 0; i < s->n; i++)
        if ((k = set_size(s->u.l.names[s->n + i])) < 0 || (n *= k) > 2147483647) return -1;
    return n;
}

static int set_member(Value *v, Value *s);

static int all_members(Value *sub, Value *s) {
    Value *e = enumerate(sub);
    for (int i = 0; i < e->n; i++) if (!set_member(e->u.set.elems[i], s)) return 0;
    return 1;
}

/* TLC's error testing membership in a value that is not a set. */
static _Noreturn void not_a_set(Value *v, Value *s) {
    if (s->kind == V_TUPLE) fail("Attempted to check set membership in a tuple value.");
    if (s->kind == V_FCN && s->made == MADE_RECORD) fail("Attempted to check if element:\n%s\nis in the record:\n%s", pretty(v), pretty(s));
    fail("Attempted to check if the value:\n%s\nis an element of the %s %s", pretty(v),
         s->kind == V_FCN || s->kind == V_LAMBDA ? "function" : kind_name(s), pretty(s));
}

static int set_member(Value *v, Value *s) {
    if (!is_set(s)) not_a_set(v, s);
    v = realize(v);
    switch (s->kind) {
    case V_SET: {
        int lo = 0, hi = s->n;
        if (v->kind == V_MODEL || (s->n && s->u.set.elems[0]->kind == V_MODEL)) {
            for (int i = 0; i < s->n; i++) if (veq(v, s->u.set.elems[i])) return 1;
            return 0;
        }
        if (s->u.set.product && v->kind == V_TUPLE) {   /* the search below, as a product of intervals */
            Value *p = s->u.set.product;
            int in = v->n == p->n;
            for (int i = 0; i < v->n; i++) {
                Value *x = v->u.fcn.vals[i], *part = in ? p->u.l.names[p->n + i] : NULL;
                if (x->kind != V_INT || (part && part->kind != V_INTERVAL)) goto search;
                in = in && x->u.i >= part->u.range.lo && x->u.i <= part->u.range.hi;
            }
            return in;
        }
    search:
        /* TLC's search: its probes decide which element an error names. */
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            int c = vcmp_quick(v, s->u.set.elems[mid]);
            if (!c) return 1;
            if (c < 0) hi = mid; else lo = mid + 1;
        }
        return 0;
    }
    case V_INTERVAL:
        if (v->kind != V_INT) {
            if (v->kind == V_MODEL || !s->n) return 0;   /* TLC finds nothing in an empty interval */
            fail("Attempted to check if the value:\n%s\nis in the integer interval %s", pretty(v), pretty(s));
        }
        return v->u.i >= s->u.range.lo && v->u.i <= s->u.range.hi;
    case V_LAZY:
        switch (s->lazy) {
        case L_NAT: case L_INT: case L_STRING:
            if (v->kind == V_MODEL) return 0;
            if (v->kind != (s->lazy == L_STRING ? V_STR : V_INT))
                fail_module("Attempted to check if the value:\n%s\nis an element of %s.", pretty(v),
                            s->lazy == L_NAT ? "Nat" : s->lazy == L_INT ? "Int" : "STRING");
            return s->lazy != L_NAT || v->u.i >= 0;
        case L_ANY: return 1;
        case L_SUBSET:
            if (!is_set(v)) fail("Attempted to check if the non-enumerable value\n%s\nis element of\n%s", pretty(v), pretty(s));
            return all_members(v, s->u.l.a);
        case L_SEQ:
            if (v->kind == V_STR) v = str_chars(v);
            if (!is_fcn(v) || !fcn_is_tuple(v))
                fail_module("Attempted to check if the value:\n%s\nis an element of %s.", pretty(v), pretty(s));
            for (int i = 0; i < v->n; i++) if (!set_member(v->u.fcn.vals[i], s->u.l.a)) return 0;
            return 1;
        case L_FCNSET: {
            if (!is_fcn(v))
                fail("Attempted to check if \n%s\nwhich is not a TLC function value, is in the set of functions:\n%s", pretty(v), pretty(s));
            Value *dom = enumerate(s->u.l.a);
            if (dom->n != v->n) return 0;
            for (int i = 0; i < v->n; i++) {
                if (!veq(fcn_key(v, i), dom->u.set.elems[i])) return 0;
                if (!set_member(v->u.fcn.vals[i], s->u.l.b)) return 0;
            }
            return 1;
        }
        case L_RCDSET: {
            if (!is_fcn(v) || !(v->n == 0 || fcn_is_record(v)))
                fail("Attempted to check if non-record\n%s\nis in the set of records:\n%s", pretty(v), pretty(s));
            if (v->kind != V_FCN || v->n != s->n) return 0;
            for (int i = 0; i < s->n; i++) {
                if (!veq(v->u.fcn.keys[i], s->u.l.names[i])) return 0;
                if (!set_member(v->u.fcn.vals[i], s->u.l.names[s->n + i])) return 0;
            }
            return 1;
        }
        case L_TIMES: {
            if (!is_fcn(v) || !fcn_is_tuple(v))
                fail("Attempted to check if non-tuple\n%s\nis in the set of tuples:\n%s", pretty(v), pretty(s));
            if (v->n != s->n) return 0;
            for (int i = 0; i < s->n; i++) if (!set_member(v->u.fcn.vals[i], s->u.l.names[s->n + i])) return 0;
            return 1;
        }
        case L_FILTER:
            if (s->u.l.b) return set_member(v, s->u.l.b);
            return set_member(v, s->u.l.a) && filter_member_test(s, v) == TRUEV;
        case L_UNION:
            union_sets(s);
            for (int i = 0; i < s->u.l.a->n; i++) if (set_member(v, s->u.l.a->u.set.elems[i])) return 1;
            return 0;
        case L_CUP: case L_CAP: case L_DIFF: {
            fp_enter(s->u.l.expr);
            int in = set_member(v, s->u.l.a);
            if (s->lazy == L_CUP ? !in : in) in = set_member(v, s->u.l.b) == (s->lazy != L_DIFF);
            fp_leave(s->u.l.expr);
            return in;
        }
        }
        return 0;
    default:
        fail("Attempted to check if the value:\n%s\nis an element of %s.", pretty(v), pretty(s));
    }
}

/* A set operation whose result may be infinite stays lazy (§7). */
static Value *lazy_op(int kind, Value *a, Value *b) {
    Value *s = mk_lazy(kind);
    s->u.l.a = a;
    s->u.l.b = b;
    return s;
}

/* A set TLC holds as its elements: one it computes filters and functions
   over at once (TLC's Reducible). */
static int reducible(Value *s) {
    return s->kind == V_INTERVAL || (s->kind == V_SET && s->made != MADE_HELD);
}

/* s, as a set TLC holds as its definition. */
static Value *held(Value *s) {
    if (s->kind != V_SET || s->made == MADE_HELD) return s;
    Value *t = NEW(Value);
    *t = *s;
    t->made = MADE_HELD;
    return t;
}

/* TLC's error making S \cup T of elements that do not compare: it looks
   for each of T's in S, comparing for equality. */
static _Noreturn void cup_error(Value *a, Value *b) {
    for (int j = 0; j < b->n; j++)
        for (int i = 0; i < a->n; i++) veq(a->u.set.elems[i], b->u.set.elems[j]);
    abort();
}

/* TLC's error enumerating S \cup T it keeps as its definition, of
   elements that do not compare: it sorts S's and then T's by binary
   insertion. */
static _Noreturn void sort_error(Value *a, Value *b) {
    a = enumerate(a); b = enumerate(b);
    Value **v = NEWN(Value *, a->n + b->n);
    memcpy(v, a->u.set.elems, a->n * sizeof *v);
    memcpy(v + a->n, b->u.set.elems, b->n * sizeof *v);
    for (int i = 1, n = 1; i < a->n + b->n; i++) {
        int lo = 0, hi = n, idx = 0, c = 0;
        while (lo < hi) {
            idx = lo + (hi - lo) / 2;
            if (!(c = vcmp(v[i], v[idx]))) break;
            if (c < 0) hi = idx; else lo = idx + 1;
        }
        if (c) {
            idx = c < 0 ? idx : idx + 1;
            Value *e = v[i];
            memmove(v + idx + 1, v + idx, (n - idx) * sizeof *v);
            v[idx] = e;
            n++;
        }
    }
    abort();
}

/* The kind of a set's elements but model values, which compare with any
   value; -1 if it has no other. */
static int elem_kind(Value *s) {
    for (int i = 0; i < s->n; i++) {
        Value *e = s->u.set.elems[i];
        if (e->kind != V_MODEL) return is_fcn(e) ? V_FCN : is_set(e) ? V_SET : e->kind;
    }
    return -1;
}

/* Whether two enumerated sets have elements that do not compare. */
static int kinds_differ(Value *a, Value *b) {
    int ka = elem_kind(a), kb = elem_kind(b);
    return ka >= 0 && kb >= 0 && ka != kb;
}

/* S \cup T of two finite sets, enumerated. */
static Value *cup_merge(Value *a, Value *b) {
    a = enumerate(a); b = enumerate(b);
    if (!b->n) return a;
    if (!a->n) return b;
    if (kinds_differ(a, b)) cup_error(a, b);
    /* Both are sorted: merge them. */
    Value *r = mk_set(a->n + b->n);
    Value **x = a->u.set.elems, **y = b->u.set.elems, **out = r->u.set.elems;
    int i = 0, j = 0, k = 0;
    while (i < a->n && j < b->n) {
        int c = vcmp(x[i], y[j]);
        out[k++] = c <= 0 ? x[i] : y[j];
        i += c <= 0;
        j += c >= 0;
    }
    while (i < a->n) out[k++] = x[i++];
    while (j < b->n) out[k++] = y[j++];
    r->n = k;
    return r;
}

/* TLC enumerates S \cup T only of two sets it holds as their elements; it
   keeps another as its definition, the enumerated first, unless that is
   empty. */
static Value *set_union(Value *a, Value *b) {
    if (reducible(a) && reducible(b)) return cup_merge(a, b);
    if (reducible(a) && !a->n) return b;
    if (reducible(b) && !b->n) return a;
    return reducible(b) ? lazy_op(L_CUP, b, a) : lazy_op(L_CUP, a, b);
}

static int enumerable(Value *s) { return is_set(s) && !set_is_infinite(s); }

/* The elements of s that are in t, or not: in order, as they are in s. */
static Value *select_members(Value *s, Value *t, int in) {
    s = enumerate(s);
    Value *r = mk_set(s->n);
    int k = 0;
    for (int i = 0; i < s->n; i++) if (set_member(s->u.set.elems[i], t) == in) r->u.set.elems[k++] = s->u.set.elems[i];
    r->n = k;
    return r;
}

/* As TLC: S \cap T and S \ T of an enumerated set are its elements in T,
   or not; others are kept as their definitions. */
static Value *set_inter(Value *a, Value *b) {
    if (reducible(a)) return select_members(a, b, 1);
    if (reducible(b)) return select_members(b, a, 1);
    return lazy_op(L_CAP, a, b);
}

static Value *set_minus(Value *a, Value *b) {
    return reducible(a) ? select_members(a, b, 0) : lazy_op(L_DIFF, a, b);
}

/* ---------------------------------------------------------------- functions */

static Value *lambda_apply(Value *f, Value *a);

/* The index of a in the domain of the function or tuple f, or -1. */
static int fcn_index(Value *f, Value *a) {
    if (f->kind == V_TUPLE) return a->kind == V_INT && a->u.i >= 1 && a->u.i <= f->n ? (int)a->u.i - 1 : -1;
    int models = f->n && f->u.fcn.keys[0]->kind == V_MODEL && f->u.fcn.keys[f->n - 1]->kind == V_MODEL;
    if (a->kind == V_MODEL ? !models : f->n && f->u.fcn.keys[0]->kind == V_MODEL) {
        /* A domain mixing model values with others: compare for equality. */
        for (int i = 0; i < f->n; i++) if (veq(a, f->u.fcn.keys[i])) return i;
        return -1;
    }
    if (f->n && (a->kind == V_INT || a->kind == V_MODEL) && f->u.fcn.keys[0]->kind == a->kind) {
        /* A domain of consecutive numbers or model values: index it. */
        int64_t i = a->u.i - f->u.fcn.keys[0]->u.i;
        if (i >= 0 && i < f->n && f->u.fcn.keys[i]->kind == a->kind && f->u.fcn.keys[i]->u.i == a->u.i) return (int)i;
    }
    /* Probing as TLC tests membership in a function's domain. */
    int lo = 0, hi = f->n;
    while (lo < hi) {
        int mid = (lo + hi) >> 1;
        int c = vcmp_quick(a, f->u.fcn.keys[mid]);
        if (!c) return mid;
        if (c < 0) hi = mid; else lo = mid + 1;
    }
    return -1;
}

/* Applies f to the argument a. */
static Value *fcn_apply(Value *f, Value *a, Expr *where) {
    if (f->kind == V_LAMBDA) return lambda_apply(f, a);
    if (!is_fcn(f)) fail_at(where, "A non-function (%s) was applied as a function.", kind_string(f));
    int i = fcn_index(f, a);
    if (i >= 0) return f->u.fcn.vals[i];
    if (f->kind == V_TUPLE) {
        fp_enter(f->u.fcn.source);   /* TLC cannot fingerprint the tuple */
        if (a->kind != V_INT) fail("Attempted to access tuple at a non integral index: %s", pretty(a));
        fail("Attempted to access index %lld of tuple\n%s\nwhich is out of bounds.", (long long)a->u.i, pretty(f));
    }
    if (f->made == MADE_RECORD) {
        if (a->kind != V_STR) fail("Attempted to access record by a non-string argument: %s", pretty(a));
        fail("Attempted to access nonexistent field '%s' of record\n%s", S(a->u.i), pretty(f));
    }
    if (f->made >= MADE_LAMBDA)
        fail("In applying the function\n%s,\nthe first argument is:\n%s\nwhich is not in its domain.\n", pretty(f), pretty(a));
    fail("Attempted to apply function:\n%s\nto argument %s, which is not in the domain of the function.",
         pretty(f), pretty(a));
}

static Value *fcn_domain(Value *f) {
    if (f->kind == V_LAMBDA) return f->u.l.a;
    if (f->kind == V_TUPLE) return mk_interval(1, f->n);
    Value *s = mk_set(f->n);
    for (int i = 0; i < f->n; i++) s->u.set.elems[i] = f->u.fcn.keys[i];
    return s;
}

typedef struct { Value *k, *v; } Pair;

static int cmp_pair(const void *x, const void *y) { return vcmp(((Pair *)x)->k, ((Pair *)y)->k); }

/* A function from parallel lists of keys and values, sorted by key. */
static Value *mk_fcn_from(List *keys, List *vals) {
    int n = keys->n;
    Pair *p = NEWN(Pair, n);
    for (int i = 0; i < n; i++) { p[i].k = keys->v[i]; p[i].v = vals->v[i]; }
    if (n > 1) qsort(p, (size_t)n, sizeof(Pair), cmp_pair);
    Value *f = mk_fcn(n);
    for (int i = 0; i < n; i++) { f->u.fcn.keys[i] = p[i].k; f->u.fcn.vals[i] = p[i].v; }
    return f;
}

/* A copy of f whose value at key is val; key must be in the domain. */
static inline uint64_t pair_hash(Value *f, int i, Value *val);

/* f with the value val at key, which is its i-th argument unless f is a lambda. */
static Value *fcn_update(Value *f, int i, Value *key, Value *val) {
    Value *g = NEW(Value);
    *g = *f;
    if (f->kind == V_LAMBDA) {
        g->n = f->n + 1;
        g->u.l.names = NEWN(Value *, 2 * g->n);
        if (f->n) memcpy(g->u.l.names, f->u.l.names, sizeof(Value *) * 2 * (size_t)f->n);
        g->u.l.names[2 * f->n] = key;
        g->u.l.names[2 * f->n + 1] = val;
        return g;
    }
    g->u.fcn.vals = alloc_raw(sizeof(Value *) * (size_t)f->n);
    memcpy(g->u.fcn.vals, f->u.fcn.vals, sizeof(Value *) * (size_t)f->n);
    g->u.fcn.vals[i] = val;
    g->u.fcn.source = NULL;   /* a tuple TLC makes anew */
    /* Kept up to date only with a hash known: TLC hashes a set when it
       fingerprints the state, where an error is its own. */
    int known = val->kind <= V_MODEL || (val->kind == V_SET ? val->u.set.hash != 0
              : (val->kind == V_TUPLE || val->kind == V_FCN) && val->u.fcn.hash != 0);
    if (f->u.fcn.hash && known) g->u.fcn.hash = f->u.fcn.hash - pair_hash(f, i, f->u.fcn.vals[i]) + pair_hash(f, i, val);
    else g->u.fcn.hash = 0;
    return g;
}

/* ---------------------------------------------------------------- syntax */

enum { T_EOF, T_IDENT, T_NUM, T_STR, T_OP, T_KW, T_SEP, T_END, T_STEP };   /* T_STEP: a proof step, <1>2. */

typedef struct {
    int kind;
    int id;              /* interned text of an identifier, numeral, operator, or keyword */
    int64_t num;
    const char *text;    /* a string literal's contents, unescaped */
    int len;
    int line, col, eline, ecol;
} Tok;

/* Expression kinds. The parser produces the X_ kinds; the resolver turns
   names and operator applications into the kinds after X_RESOLVED. */
enum {
    X_NUM, X_STR, X_BOOL, X_NAME, X_OP, X_AT, X_LAMBDA, X_OPREF,
    X_SETENUM, X_TUPLE, X_RECORD, X_RCDSET, X_FCNSET, X_FCN, X_APPLY, X_DOT, X_EXCEPT,
    X_FILTER, X_MAP, X_FORALL, X_EXISTS, X_CHOOSE, X_IF, X_CASE, X_LET,
    X_SQACT, X_ANGLEACT, X_WF, X_SF, X_TFORALL, X_TEXISTS,
    X_RESOLVED,
    /* resolved */
    K_BOUND,     /* a bound name or operator parameter: def */
    K_DECL,      /* a constant or variable: def */
    K_CALL,      /* a definition applied to args: def */
    K_PARAMCALL, /* an operator parameter applied to args: def */
    K_INSTCALL,  /* a definition reached through instances: path in defs, final def */
    K_BOOLEAN, K_STRINGSET,
    K_AND, K_OR, K_NOT, K_IMPLIES, K_EQUIV, K_EQ, K_NEQ, K_IN, K_NOTIN,
    K_CUP, K_CAP, K_SETMINUS, K_SUBSETEQ, K_SUBSET, K_UNION, K_DOMAIN,
    K_PRIME, K_UNCHANGED, K_ENABLED, K_ALWAYS, K_EVENTUALLY, K_LEADSTO, K_TIMES, K_CDOT, K_WHILE
};

typedef struct {
    int n;
    int *names;
    Def **defs;     /* filled by the resolver */
    int tuple;      /* <<a, b>> \in S */
    Expr *set;      /* NULL for an unbounded binding */
    int line, col;  /* of its first name */
} Bound;

/* A segment of A!B(x)!C, or a subexpression selector (§5.7): !2, !<<, !>>,
   !:, !@ and !(x) are sel > 0 or the SEL_ kinds; a name is sel 0. */
typedef struct { int name; int sel; Expr **args; int n; } Seg;
enum { SEL_LL = -1, SEL_GG = -2, SEL_COLON = -3, SEL_AT = -4, SEL_NULL = -5 };

/* lab(x, y) :: e names e, with the bound identifiers x and y, for selectors. */
typedef struct { int name; int n; int *names; Def **defs; } Label;

typedef struct { int n; Expr **keys; int *fields; int line, col; } Path;  /* an EXCEPT path: fields[i] >= 0 for .f; line and col of its ! */

struct Expr {
    short kind;
    unsigned char fixed;  /* fixed_expr: 0 not yet known, 1 yes, 2 no */
    int op;               /* operator or name; field of X_DOT */
    int n;
    Expr **args;
    Bound *bounds;
    int nb;
    Def *def;
    Def **defs;           /* LET definitions; instance path of K_INSTCALL */
    int ndefs;
    Seg *segs;            /* X_NAME: A!B(x)!C */
    Label *label;
    int nseg;
    Path *paths;          /* X_EXCEPT */
    int64_t num;
    const char *text;
    int len;
    Module *mod;
    int l1, c1, l2, c2;
    int pl1, pc1, pl2, pc2;   /* with the parentheses around it, if any */
    Value *value;         /* X_SETENUM of literals of one kind, or a fixed product: its value, once computed */
};

enum { D_OP, D_FCN, D_CONST, D_VAR, D_BOUND, D_PARAM, D_INST, D_RECURSIVE, D_THM };   /* D_THM: a theorem's or assumption's name */

typedef Value *(*Native)(Expr *call, Value **args, Ctx c);

struct Def {
    int kind;
    int name;
    int arity;            /* of an operator parameter or declared constant: F(_, _) */
    int nparams;
    Def **params;
    Expr *body;
    Bound *bounds;        /* D_FCN: f[x \in S] == body */
    int nb;
    Module *mod;
    Native native;
    const char *java;     /* a native's Java method in TLC, as TLC prints it */
    int var_index;        /* D_VAR of the root: index in the state, or -1 */
    Value *value;         /* the configured value of a constant or definition */
    Def *replacement;     /* D_CONST or D_OP replaced by an operator of the configuration */
    int bad_target;       /* the undefined name the configuration replaces it by */
    int level;            /* 0 constant, 1 state, 2 action, 3 temporal; -1 unknown */
    int level_busy;
    Def *level_probe;     /* a LET definition's: the parameter probed when its level was found */
    unsigned level_mask;  /* 1 + twice the bits of the parameters a call's level depends on; 0 unknown */
    int level_bound;      /* TLC's bound on its level, plus 1; 0 unknown */
    int level_max;        /* a parameter's or declaration's: the highest level its uses allow, plus 1; 0 for any */
    int level_checked;
    Value *cache;         /* a constant definition without parameters, once evaluated */
    int uncached;         /* one that failed as TLC started: evaluated each time, as in TLC */
    int local;
    int recursive;        /* a LET definition declared RECURSIVE: in scope before it */
    int self;             /* D_FCN whose body names it: TLC's RecursiveFcnSpec, always lazy (§6.6) */
    Module *target;       /* D_INST */
    int nsubst;
    Def **subst_decl;
    Expr **subst_expr;
    Expr *where;          /* the definition's location */
};

static Value *call_native(Def *d, Expr *call, Value **args, Ctx c);

/* A symbol of a module: a definition, reached through `npath` instances. */
typedef struct { int name; Def *def; Def **path; int npath; int local; } Sym;

typedef struct {
    int kind;             /* 0 definition, 1 instance, 2 assume, 3 nested module, 4 name only */
    Def *def;
    Expr *expr;
    Module *module;
    int local;
} Unit;

struct Module {
    int name;
    char *path;
    Module *outer;        /* a nested module's enclosing module */
    int nouter_syms;      /* the enclosing module's symbols visible to it */
    Buf warnings;         /* a file's module: SANY's warnings for it, nested modules' too */
    int nwarnings;
    int *extends;
    int nextends;
    Sym *syms;
    int nsyms, capsyms;
    Def **decls;          /* constants and variables, own and extended, in order */
    int ndecls, capdecls;
    Unit *units;
    int nunits, capunits;
    Module **nested;
    int nnested;
    Tok *toks;
    int ntoks;
    int resolved;         /* 1 while resolving, 2 after */
    int standard;         /* read from the text of a standard module */
    int instantiated;     /* the target of an INSTANCE */
    Expr *odd[3];         /* its first decimal or too big integer: in a definition, an assumption, a theorem */
};

/* ---------------------------------------------------------------- lexer */

static const char *keywords[] = {
    "ASSUME", "ASSUMPTION", "AXIOM", "BOOLEAN", "CASE", "CHOOSE", "CONSTANT", "CONSTANTS", "DOMAIN", "ELSE",
    "ENABLED", "EXCEPT", "EXTENDS", "FALSE", "IF", "IN", "INSTANCE", "LET", "LOCAL", "MODULE", "OTHER",
    "STRING", "SUBSET", "THEN", "THEOREM", "TRUE", "UNCHANGED", "UNION", "VARIABLE", "VARIABLES",
    "WITH", "WF_", "SF_", "LAMBDA", "RECURSIVE", "LEMMA", "PROPOSITION", "COROLLARY", "PROOF", "BY",
    "OBVIOUS", "OMITTED", "QED", "USE", "HIDE", "DEFS", "DEF", "SUFFICES", "HAVE", "TAKE", "WITNESS",
    "PICK", "NEW", "ACTION", "STATE", "TEMPORAL", "ONLY", NULL};

static const char *symbols[] = {
    "(\\X)", "-+->", "<=>", "...", "::=", "|->", "(+)", "(-)", "(.)", "(/)",
    "<<", ">>", "==", "/=", "<=", ">=", "=<", "=>", "->", "<-", "/\\", "\\/", "..", "::", ":=", ":>",
    "<:", "@@", "~>", "[]", "<>", "++", "--", "**", "//", "^^", "%%", "&&", "||", "$$", "??", "!!",
    "##", "|-", "-|", "|=", "=|", "^+", "^*", "^#", "-.",
    "(", ")", "[", "]", "{", "}", ",", ":", ".", "!", "@", "'", "=", "#", "<", ">", "+", "-", "*",
    "/", "%", "^", "&", "|", "$", "?", "~", "_", "\\", NULL};

static int is_keyword(const char *s, int len) {
    for (int i = 0; keywords[i]; i++)
        if ((int)strlen(keywords[i]) == len && memcmp(keywords[i], s, (size_t)len) == 0) return 1;
    return 0;
}

static int id_char(int c) { return isalnum(c) || c == '_'; }

typedef struct { const char *src; const char *file; int pos, line, col; } Lexer;

static _Noreturn void parse_error(const char *file, int line, int col, const char *fmt, ...);

static void lex_advance(Lexer *L, int n) {
    while (n--) {
        if (L->src[L->pos] == '\n') { L->line++; L->col = 1; }
        else L->col++;
        L->pos++;
    }
}

static int starts_module(Lexer *L) {
    const char *s = L->src + L->pos;
    int i = 0;
    while (s[i] == '-') i++;
    if (i < 4) return 0;
    while (s[i] == ' ' || s[i] == '\t') i++;
    return strncmp(s + i, "MODULE", 6) == 0 && !id_char(s[i + 6]);
}

/* Skips white space and comments. */
static void lex_space(Lexer *L) {
    for (;;) {
        char c = L->src[L->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') { lex_advance(L, 1); continue; }
        if (c == '\\' && L->src[L->pos + 1] == '*') {
            while (L->src[L->pos] && L->src[L->pos] != '\n') lex_advance(L, 1);
            continue;
        }
        if (c == '(' && L->src[L->pos + 1] == '*') {
            int depth = 0;
            do {
                if (!L->src[L->pos]) parse_error(L->file, L->line, L->col, "Unterminated comment");
                if (L->src[L->pos] == '(' && L->src[L->pos + 1] == '*') { depth++; lex_advance(L, 2); }
                else if (L->src[L->pos] == '*' && L->src[L->pos + 1] == ')') { depth--; lex_advance(L, 2); }
                else lex_advance(L, 1);
            } while (depth > 0);
            continue;
        }
        return;
    }
}

static int digit_value(int c) {
    if (isdigit(c)) return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 99;
}

/* The tokens of a file's outermost module, interning identifiers and
   numerals in the order they appear (§7.5). */
/* The length of the proof step s begins with, as <1>, <2>3., <*>; 0 if none. */
static int proof_step(const char *s) {
    if (s[0] != '<') return 0;
    int len = isdigit((unsigned char)s[1]) ? 1 + (int)strspn(s + 1, "0123456789") : s[1] == '*' || s[1] == '+' ? 2 : 0;
    if (!len || s[len] != '>') return 0;
    for (len++; id_char(s[len]); len++) {}
    while (s[len] == '.') len++;
    return len;
}

static Tok *lex(const char *src, const char *file, int *count) {
    Lexer L = {src, file, 0, 1, 1};
    Tok *toks = NULL;
    int n = 0, cap = 0, depth = 0;
    /* Text before the module is ignored (§3.2). */
    for (;;) {
        if (!src[L.pos]) parse_error(file, L.line, L.col, "Could not find a module");
        if ((L.pos == 0 || src[L.pos - 1] == '\n' || 1) && src[L.pos] == '-' && starts_module(&L)) break;
        lex_advance(&L, 1);
    }
    for (;;) {
        lex_space(&L);
        if (n + 2 >= cap) { cap = cap ? cap * 2 : 1024; toks = xrealloc(toks, sizeof(Tok) * (size_t)cap); }
        Tok *t = &toks[n];
        memset(t, 0, sizeof *t);
        t->line = L.line; t->col = L.col;
        const char *s = src + L.pos;
        if (!*s) { t->kind = T_EOF; n++; break; }
        int len = 0;
        if (s[0] == '-' && s[1] == '-' && s[2] == '-' && s[3] == '-') {
            while (s[len] == '-') len++;
            t->kind = T_SEP;
            if (starts_module(&L)) depth++;
        } else if (s[0] == '=' && s[1] == '=' && s[2] == '=' && s[3] == '=') {
            while (s[len] == '=') len++;
            t->kind = T_END;
            depth--;
        } else if (id_char(s[0])) {
            while (id_char(s[len])) len++;
            int digits = 1;
            for (int i = 0; i < len; i++) if (!isdigit((unsigned char)s[i])) digits = 0;
            if (digits) {
                t->kind = T_NUM;
                int64_t v = 0;
                for (int i = 0; i < len; i++) { v = v * 10 + (s[i] - '0'); if (v > 0x7fffffffffffLL) v = 0x7fffffffffffLL; }
                t->num = v;
                if (s[len] == '.' && isdigit((unsigned char)s[len + 1])) {   /* a decimal: -1 */
                    for (len++; isdigit((unsigned char)s[len]); len++) {}
                    t->num = -1;
                }
                t->id = intern_n(s, len);
            } else if ((len > 3 && (strncmp(s, "WF_", 3) == 0 || strncmp(s, "SF_", 3) == 0))) {
                len = 3;
                t->kind = T_KW;
                t->id = intern_n(s, 3);
            } else if (is_keyword(s, len)) {
                t->kind = T_KW;
                t->id = intern_n(s, len);
            } else if (len == 1 && s[0] == '_') {
                t->kind = T_OP;
                t->id = intern("_");
            } else {
                t->kind = T_IDENT;
                t->id = intern_n(s, len);
            }
        } else if (s[0] == '"') {
            Buf b = {0};
            len = 1;
            while (s[len] != '"') {
                if (!s[len] || s[len] == '\n') parse_error(file, t->line, t->col, "Unterminated string");
                if (s[len] == '\\') {
                    char e = s[len + 1];
                    char c = e == 'n' ? '\n' : e == 't' ? '\t' : e == 'r' ? '\r' : e == 'f' ? '\f' : e;
                    buf_add(&b, &c, 1);
                    len += 2;
                } else {
                    buf_add(&b, s + len, 1);
                    len++;
                }
            }
            len++;
            t->kind = T_STR;
            t->len = (int)b.n;
            t->text = b.s ? b.s : "";
        } else if (s[0] == '\\' && isalnum((unsigned char)s[1])) {
            len = 1;
            while (isalnum((unsigned char)s[len])) len++;
            int base = strchr("bB", s[1]) ? 2 : strchr("oO", s[1]) ? 8 : strchr("hH", s[1]) ? 16 : 0;
            int ok = base && len > 2;
            for (int i = 2; ok && i < len; i++) if (digit_value(s[i]) >= base) ok = 0;
            if (ok) {
                t->kind = T_NUM;
                int64_t v = 0;
                for (int i = 2; i < len; i++) { v = v * base + digit_value(s[i]); if (v > 0x7fffffffffffLL) v = 0x7fffffffffffLL; }
                t->num = v;
                t->id = intern_n(s, len);
            } else {
                t->kind = T_OP;
                t->id = intern_n(s, len);
            }
        } else if ((len = proof_step(s))) {
            t->kind = T_STEP;
            t->id = intern_n(s, len);
        } else {
            for (int i = 0; symbols[i]; i++) {
                int l = (int)strlen(symbols[i]);
                if (strncmp(s, symbols[i], (size_t)l) == 0) { len = l; break; }
            }
            if (!len) parse_error(file, t->line, t->col, "Unexpected character '%c'", s[0]);
            /* >>_ and ]_ begin a subscript: <<A>>_v and [A]_v. */
            if ((len == 2 && s[0] == '>' && s[1] == '>' && s[2] == '_') || (len == 1 && s[0] == ']' && s[1] == '_')) len++;
            t->kind = T_OP;
            t->id = intern_n(s, len);
        }
        lex_advance(&L, len - 1);
        t->eline = L.line; t->ecol = L.col;
        lex_advance(&L, 1);
        n++;
        if (t->kind == T_END && depth == 0) {
            toks[n].kind = T_EOF; toks[n].line = L.line; toks[n].col = L.col; n++;
            break;
        }
    }
    *count = n;
    return toks;
}

/* ---------------------------------------------------------------- parser */

typedef struct {
    Tok *t;
    int pos, n;
    int limit;        /* tokens at or left of this column end the junction item (§6.2) */
    Module *mod;
    const char *file;
} Parser;

static Parser *P;

static _Noreturn void parse_error(const char *file, int line, int col, const char *fmt, ...) {
    char msg[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    printf("***Parse Error***\n%s at line %d, column %d in %s\n\n", msg, line, col, file);
    printf("Error: Parsing or semantic analysis failed.\n");
    exit(TPP_ERROR_PARSE);
}

static Tok eof_tok;

static Tok *peek(void) {
    Tok *t = &P->t[P->pos];
    if (t->kind != T_EOF && t->col <= P->limit) { eof_tok = *t; eof_tok.kind = T_EOF; return &eof_tok; }
    return t;
}

static Tok *peek_at(int k) {
    if (P->pos + k >= P->n) return &P->t[P->n - 1];
    return &P->t[P->pos + k];
}

static Tok *prev_tok(void) { return &P->t[P->pos - 1]; }

static Tok *next(void) { Tok *t = &P->t[P->pos]; if (t->kind != T_EOF) P->pos++; return t; }

static _Noreturn void syntax_error(const char *what) {
    Tok *t = &P->t[P->pos];
    const char *text = t->kind == T_EOF ? "end of module" : t->kind == T_STR ? "string" : S(t->id);
    parse_error(P->file, t->line, t->col, "Encountered \"%s\": %s", text, what);
}

static int is_op(Tok *t, const char *s) { return t->kind == T_OP && strcmp(S(t->id), s) == 0; }
static int is_kw(Tok *t, const char *s) { return t->kind == T_KW && strcmp(S(t->id), s) == 0; }

static void expect_op(const char *s) { if (!is_op(peek(), s)) syntax_error(s); next(); }
static void expect_kw(const char *s) { if (!is_kw(peek(), s)) syntax_error(s); next(); }

static int expect_ident(void) {
    Tok *t = peek();
    if (t->kind != T_IDENT) syntax_error("expected an identifier");
    next();
    return t->id;
}

/* A record field: any word, keywords included. */
static int is_field(Tok *t) { return t->kind == T_IDENT || (t->kind == T_KW && isalpha((unsigned char)S(t->id)[0])); }

static int expect_field(void) {
    Tok *t = peek();
    if (!is_field(t)) syntax_error("expected a field name");
    next();
    return t->id;
}

static Expr *mk_expr(int kind, Tok *start) {
    Expr *e = NEW(Expr);
    e->kind = (short)kind;
    e->mod = P->mod;
    e->l1 = start->line; e->c1 = start->col;
    e->l2 = start->eline; e->c2 = start->ecol;
    return e;
}

static void finish(Expr *e) {
    Tok *t = prev_tok();
    e->l2 = t->eline; e->c2 = t->ecol;
}

/* e starts where a does, or at the parenthesis before it. */
static void span_from(Expr *e, Expr *a) {
    if (a->pl1) { e->l1 = a->pl1; e->c1 = a->pc1; } else { e->l1 = a->l1; e->c1 = a->c1; }
}

static void span_to(Expr *e, Expr *a) {
    if (a->pl1) { e->l2 = a->pl2; e->c2 = a->pc2; } else { e->l2 = a->l2; e->c2 = a->c2; }
}

static Expr **args_array(Expr **a, int n) {
    Expr **r = NEWN(Expr *, n);
    for (int i = 0; i < n; i++) r[i] = a[i];
    return r;
}

/* Operators (§6.1). */
typedef struct { const char *name; int lo, hi, left, fix; } OpInfo;
enum { INFIX, PREFIX, POSTFIX };

static const OpInfo op_table[] = {
    {"=>", 1, 1, 0, INFIX}, {"~>", 2, 2, 0, INFIX}, {"<=>", 2, 2, 0, INFIX}, {"-+->", 2, 2, 0, INFIX},
    {"/\\", 3, 3, 1, INFIX}, {"\\/", 3, 3, 1, INFIX},
    {"=", 5, 5, 0, INFIX}, {"/=", 5, 5, 0, INFIX}, {"<", 5, 5, 0, INFIX}, {">", 5, 5, 0, INFIX},
    {"\\leq", 5, 5, 0, INFIX}, {"\\geq", 5, 5, 0, INFIX}, {"\\in", 5, 5, 0, INFIX},
    {"\\notin", 5, 5, 0, INFIX}, {"\\subseteq", 5, 5, 0, INFIX}, {"\\subset", 5, 5, 0, INFIX},
    {"\\supseteq", 5, 5, 0, INFIX}, {"\\supset", 5, 5, 0, INFIX}, {"\\prec", 5, 5, 0, INFIX},
    {"\\succ", 5, 5, 0, INFIX}, {"\\preceq", 5, 5, 0, INFIX}, {"\\succeq", 5, 5, 0, INFIX},
    {"\\sim", 5, 5, 0, INFIX}, {"\\simeq", 5, 5, 0, INFIX}, {"\\ll", 5, 5, 0, INFIX},
    {"\\gg", 5, 5, 0, INFIX}, {"\\asymp", 5, 5, 0, INFIX}, {"\\approx", 5, 5, 0, INFIX},
    {"\\cong", 5, 5, 0, INFIX}, {"\\sqsubset", 5, 5, 0, INFIX}, {"\\sqsubseteq", 5, 5, 0, INFIX},
    {"\\sqsupset", 5, 5, 0, INFIX}, {"\\sqsupseteq", 5, 5, 0, INFIX}, {"\\doteq", 5, 5, 0, INFIX},
    {"\\propto", 5, 5, 0, INFIX}, {":=", 5, 5, 0, INFIX}, {"::=", 5, 5, 0, INFIX},
    {"|-", 5, 5, 0, INFIX}, {"-|", 5, 5, 0, INFIX}, {"|=", 5, 5, 0, INFIX}, {"=|", 5, 5, 0, INFIX},
    {"\\cdot", 5, 14, 1, INFIX}, {"@@", 6, 6, 1, INFIX}, {":>", 7, 7, 0, INFIX}, {"<:", 7, 7, 0, INFIX},
    {"\\", 8, 8, 0, INFIX}, {"\\cap", 8, 8, 1, INFIX}, {"\\cup", 8, 8, 1, INFIX},
    {"..", 9, 9, 0, INFIX}, {"...", 9, 9, 0, INFIX}, {"!!", 9, 13, 0, INFIX}, {"##", 9, 13, 1, INFIX},
    {"$", 9, 13, 1, INFIX}, {"$$", 9, 13, 1, INFIX}, {"??", 9, 13, 1, INFIX}, {"\\uplus", 9, 13, 1, INFIX},
    {"\\sqcap", 9, 13, 1, INFIX}, {"\\sqcup", 9, 13, 1, INFIX}, {"\\wr", 9, 14, 0, INFIX},
    {"\\X", 10, 13, 1, INFIX}, {"+", 10, 10, 1, INFIX}, {"++", 10, 10, 1, INFIX},
    {"\\oplus", 10, 10, 1, INFIX}, {"%", 10, 11, 0, INFIX}, {"%%", 10, 11, 1, INFIX},
    {"|", 10, 11, 1, INFIX}, {"||", 10, 11, 1, INFIX}, {"-", 11, 11, 1, INFIX}, {"--", 11, 11, 1, INFIX},
    {"\\ominus", 11, 11, 1, INFIX}, {"*", 13, 13, 1, INFIX}, {"**", 13, 13, 1, INFIX},
    {"&", 13, 13, 1, INFIX}, {"&&", 13, 13, 1, INFIX}, {"\\o", 13, 13, 1, INFIX},
    {"\\odot", 13, 13, 1, INFIX}, {"\\otimes", 13, 13, 1, INFIX}, {"\\star", 13, 13, 1, INFIX},
    {"\\bigcirc", 13, 13, 1, INFIX}, {"\\bullet", 13, 13, 1, INFIX}, {"/", 13, 13, 0, INFIX},
    {"//", 13, 13, 0, INFIX}, {"\\div", 13, 13, 0, INFIX}, {"\\oslash", 13, 13, 0, INFIX},
    {"^", 14, 14, 0, INFIX}, {"^^", 14, 14, 0, INFIX},
    {"~", 4, 4, 0, PREFIX}, {"[]", 4, 15, 0, PREFIX}, {"<>", 4, 15, 0, PREFIX},
    {"ENABLED", 4, 15, 0, PREFIX}, {"UNCHANGED", 4, 15, 0, PREFIX}, {"SUBSET", 10, 13, 0, PREFIX},
    {"UNION", 10, 13, 0, PREFIX}, {"DOMAIN", 10, 13, 0, PREFIX}, {"-.", 12, 12, 0, PREFIX},
    {"'", 15, 15, 0, POSTFIX}, {"^+", 15, 15, 0, POSTFIX}, {"^*", 15, 15, 0, POSTFIX}, {"^#", 15, 15, 0, POSTFIX},
    {NULL, 0, 0, 0, 0}};

/* The canonical name of an operator symbol: synonyms map to one name. */
static int canonical(int id) {
    static const char *syn[][2] = {
        {"\\land", "/\\"}, {"\\lor", "\\/"}, {"\\lnot", "~"}, {"\\neg", "~"}, {"\\equiv", "<=>"},
        {"#", "/="}, {"\\intersect", "\\cap"}, {"\\union", "\\cup"}, {"\\times", "\\X"},
        {"<=", "\\leq"}, {"=<", "\\leq"}, {">=", "\\geq"}, {"\\circ", "\\o"},
        {"(+)", "\\oplus"}, {"(-)", "\\ominus"}, {"(.)", "\\odot"}, {"(/)", "\\oslash"},
        {"(\\X)", "\\otimes"}, {NULL, NULL}};
    for (int i = 0; syn[i][0]; i++) if (strcmp(S(id), syn[i][0]) == 0) return intern(syn[i][1]);
    return id;
}

static const OpInfo *op_info(Tok *t, int fix) {
    if (t->kind != T_OP && t->kind != T_KW) return NULL;
    const char *name = S(canonical(t->id));
    if (fix == PREFIX && strcmp(name, "-") == 0) name = "-.";
    for (int i = 0; op_table[i].name; i++)
        if (op_table[i].fix == fix && strcmp(op_table[i].name, name) == 0) return &op_table[i];
    return NULL;
}

static Expr *parse_expr(void);
static Expr *parse_binary(int bound);
static Def *parse_definition(int local);
static void parse_bounds(Expr *e, int allow_unbounded);

static Expr *mk_op(Tok *start, int op, int n, Expr **args) {
    Expr *e = mk_expr(X_OP, start);
    e->op = op;
    e->n = n;
    e->args = args_array(args, n);
    return e;
}

static int is_junction(Tok *t) { return is_op(t, "/\\") || is_op(t, "\\/") || is_op(t, "\\land") || is_op(t, "\\lor"); }

/* LAMBDA x, y : e, where an operator argument may be. */
static Expr *parse_lambda(void) {
    Expr *l = mk_expr(X_LAMBDA, next());
    Def *params[16];
    int np = 0;
    do {
        if (np == 16) syntax_error("too many parameters");
        Def *d = NEW(Def);
        d->kind = D_PARAM;
        d->name = expect_ident();
        params[np++] = d;
    } while (is_op(peek(), ",") && next());
    expect_op(":");
    Def *op = NEW(Def);
    op->kind = D_OP;
    op->name = intern("LAMBDA");
    op->nparams = np;
    op->params = NEWN(Def *, np);
    memcpy(op->params, params, sizeof(Def *) * (size_t)np);
    op->body = parse_expr();
    op->mod = P->mod;
    op->level = -1;
    op->var_index = -1;
    l->def = op;
    finish(l);
    return l;
}

/* Arguments of an operator application: expressions, LAMBDAs, or
   operator symbols passed as operators. */
static Expr **parse_args(int *count) {
    Expr **args = NULL;
    int n = 0;
    int saved = P->limit;
    P->limit = 0;
    expect_op("(");
    for (;;) {
        Tok *t = peek();
        if (is_kw(t, "LAMBDA")) {
            PUSH(args, n, parse_lambda());
        } else if ((t->kind == T_OP || t->kind == T_KW) && (op_info(t, INFIX) || op_info(t, PREFIX) || op_info(t, POSTFIX)) &&
                   (is_op(peek_at(1), ",") || is_op(peek_at(1), ")"))) {
            Expr *r = mk_expr(X_OPREF, t);
            r->op = canonical(t->id);
            next();
            PUSH(args, n, r);
        } else {
            PUSH(args, n, parse_expr());
        }
        if (is_op(peek(), ",")) { next(); continue; }
        expect_op(")");
        break;
    }
    P->limit = saved;
    *count = n;
    return args;
}

/* A name, its arguments, and instance selections: A!B(x)!C. */
static Expr *parse_name(void) {
    Tok *start = peek();
    Expr *e = mk_expr(X_NAME, start);
    Seg segs[32];
    int n = 0;
    for (;;) {
        Tok *t = peek();
        Seg *s = &segs[n++];
        memset(s, 0, sizeof *s);
        static const char *sels[] = {"<<", ">>", ":", "@", "("};
        for (int k = 0; k < 5 && n > 1; k++) if (is_op(t, sels[k])) s->sel = -1 - k;
        if (n > 1 && t->kind == T_NUM && t->num > 0) s->sel = (int)t->num;
        if (s->sel == SEL_NULL) {
            s->args = parse_args(&s->n);
        } else if (s->sel) {
            next();
        } else {
            if (t->kind == T_IDENT) s->name = t->id;
            else if (t->kind == T_OP || t->kind == T_KW) s->name = canonical(t->id);
            else syntax_error("expected a name");
            next();
            if (is_op(peek(), "(")) s->args = parse_args(&s->n);
        }
        if (is_op(peek(), "!") && n < 32) { next(); continue; }
        break;
    }
    e->segs = NEWN(Seg, n);
    memcpy(e->segs, segs, sizeof(Seg) * (size_t)n);
    e->nseg = n;
    while (segs[n - 1].sel) n--;
    e->op = segs[n - 1].name;
    finish(e);
    return e;
}

/* `x, y \in S, <<a, b>> \in T` or, unbounded, `x, y`. */
static void parse_bounds(Expr *e, int allow_unbounded) {
    for (;;) {
        Bound b = {.tuple = is_op(peek(), "<<")};
        if (b.tuple) next();
        b.line = peek()->line;
        b.col = peek()->col;
        if (b.tuple) {
            do PUSH(b.names, b.n, expect_ident()); while (is_op(peek(), ",") && next());
            expect_op(">>");
        } else {
            PUSH(b.names, b.n, expect_ident());
            while (is_op(peek(), ",") && peek_at(1)->kind == T_IDENT) {
                next();
                PUSH(b.names, b.n, expect_ident());
            }
        }
        if (is_op(peek(), "\\in")) {
            next();
            b.set = parse_expr();
        } else if (!allow_unbounded) {
            syntax_error("expected \\in");
        }
        PUSH(e->bounds, e->nb, b);
        if (!is_op(peek(), ",")) break;
        next();
    }
}

/* Whether the tokens ahead are `x \in` or `x, y \in` or `<<x, y>> \in`. */
static int binder_ahead(void) {
    int k = 0;
    if (is_op(peek_at(0), "<<")) {
        k = 1;
        while (peek_at(k)->kind == T_IDENT && is_op(peek_at(k + 1), ",")) k += 2;
        if (peek_at(k)->kind != T_IDENT || !is_op(peek_at(k + 1), ">>")) return 0;
        return is_op(peek_at(k + 2), "\\in");
    }
    while (peek_at(k)->kind == T_IDENT && is_op(peek_at(k + 1), ",")) k += 2;
    return peek_at(k)->kind == T_IDENT && is_op(peek_at(k + 1), "\\in");
}

static _Noreturn void semantic_error(Expr *where, const char *fmt, ...);

static Expr *parse_junction(Tok *bullet) {
    int col = bullet->col;
    int op = canonical(bullet->id);
    int saved = P->limit;
    Expr *list = mk_expr(X_OP, bullet);
    for (;;) {
        Tok *t = &P->t[P->pos];
        if (t->kind != T_OP || canonical(t->id) != op || t->col != col) break;
        next();
        P->limit = col;
        PUSH(list->args, list->n, parse_expr());
        P->limit = saved;
    }
    list->op = op;
    finish(list);
    return list;
}

/* The v of WF_v(A), [A]_v and <<A>>_v: a name, I!v, or a tuple, set,
   bracketed or parenthesized expression, as SANY's ReducedExpression. */
static Expr *parse_subscript(void) {
    Tok *t = peek();
    if (t->kind == T_IDENT) {
        Expr *e = mk_expr(X_NAME, t);
        Seg segs[32];
        int n = 0;
        do segs[n++] = (Seg){.name = expect_ident()};
        while (n < 32 && is_op(peek(), "!") && peek_at(1)->kind == T_IDENT && next());
        e->segs = NEWN(Seg, n);
        memcpy(e->segs, segs, sizeof(Seg) * (size_t)n);
        e->nseg = n;
        e->op = segs[n - 1].name;
        finish(e);
        return e;
    }
    if (is_op(t, "<<") || is_op(t, "(") || is_op(t, "{") || is_op(t, "[")) return parse_binary(16);
    syntax_error("expected a subscript");
}

static int unit_section;   /* the part of the module being parsed, as Module.odd counts them */

static Expr *parse_primary(void) {
    Tok *t = peek();
    Expr *e;
    switch (t->kind) {
    case T_NUM:
        next();
        e = mk_expr(X_NUM, t);
        e->num = t->num;
        e->op = t->id;
        if ((e->num < 0 || e->num > 2147483647) && !P->mod->odd[unit_section]) P->mod->odd[unit_section] = e;
        return e;
    case T_STR:
        next();
        e = mk_expr(X_STR, t);
        e->text = t->text;
        e->len = t->len;
        return e;
    case T_IDENT:
        return parse_name();
    case T_KW:
        if (is_kw(t, "TRUE") || is_kw(t, "FALSE")) {
            next();
            e = mk_expr(X_BOOL, t);
            e->num = is_kw(t, "TRUE");
            return e;
        }
        if (is_kw(t, "BOOLEAN") || is_kw(t, "STRING")) {
            next();
            e = mk_expr(X_OP, t);
            e->op = t->id;
            return e;
        }
        if (is_kw(t, "IF")) {
            next();
            e = mk_expr(X_IF, t);
            Expr *a[3];
            a[0] = parse_expr();
            expect_kw("THEN");
            a[1] = parse_expr();
            expect_kw("ELSE");
            a[2] = parse_expr();
            e->n = 3;
            e->args = args_array(a, 3);
            finish(e);
            return e;
        }
        if (is_kw(t, "CASE")) {
            next();
            e = mk_expr(X_CASE, t);
            e->num = 0;
            for (;;) {
                if (is_kw(peek(), "OTHER")) {
                    next();
                    expect_op("->");
                    PUSH(e->args, e->n, NULL);
                    PUSH(e->args, e->n, parse_expr());
                    e->num = 1;
                    break;
                }
                PUSH(e->args, e->n, parse_expr());
                expect_op("->");
                PUSH(e->args, e->n, parse_expr());
                if (!is_op(peek(), "[]")) break;
                next();
            }
            finish(e);
            return e;
        }
        if (is_kw(t, "LET")) {
            next();
            e = mk_expr(X_LET, t);
            int *recursive = NULL, nrec = 0;
            while (!is_kw(peek(), "IN")) {
                if (is_kw(peek(), "RECURSIVE")) {
                    next();
                    do {
                        int name = expect_ident();
                        PUSH(recursive, nrec, name);
                        if (is_op(peek(), "(")) {
                            next();
                            while (!is_op(peek(), ")")) next();
                            next();
                        }
                    } while (is_op(peek(), ",") && next());
                    continue;
                }
                PUSH(e->defs, e->ndefs, parse_definition(0));
            }
            next();
            for (int i = 0; i < e->ndefs; i++)
                for (int k = 0; k < nrec; k++) e->defs[i]->recursive |= e->defs[i]->name == recursive[k];
            e->n = 1;
            e->args = NEWN(Expr *, 1);
            e->args[0] = parse_expr();
            finish(e);
            return e;
        }
        if (is_kw(t, "CHOOSE")) {
            next();
            e = mk_expr(X_CHOOSE, t);
            parse_bounds(e, 1);
            expect_op(":");
            e->n = 1;
            e->args = NEWN(Expr *, 1);
            e->args[0] = parse_expr();
            finish(e);
            return e;
        }
        if (is_kw(t, "WF_") || is_kw(t, "SF_")) {
            next();
            e = mk_expr(is_kw(t, "WF_") ? X_WF : X_SF, t);
            Expr *a[2];
            a[0] = parse_subscript();
            expect_op("(");
            int saved = P->limit;
            P->limit = 0;
            a[1] = parse_expr();
            P->limit = saved;
            expect_op(")");
            e->n = 2;
            e->args = args_array(a, 2);
            finish(e);
            return e;
        }
        break;
    case T_OP:
        if (is_op(t, "\\A") || is_op(t, "\\E") || is_op(t, "\\AA") || is_op(t, "\\EE") ||
            is_op(t, "\\forall") || is_op(t, "\\exists")) {
            next();
            int all = is_op(t, "\\A") || is_op(t, "\\forall") || is_op(t, "\\AA");
            int temporal = is_op(t, "\\AA") || is_op(t, "\\EE");
            e = mk_expr(temporal ? (all ? X_TFORALL : X_TEXISTS) : all ? X_FORALL : X_EXISTS, t);
            parse_bounds(e, 1);
            expect_op(":");
            e->n = 1;
            e->args = NEWN(Expr *, 1);
            e->args[0] = parse_expr();
            finish(e);
            return e;
        }
        if (is_op(t, "(")) {
            next();
            int saved = P->limit;
            P->limit = 0;
            e = parse_expr();
            P->limit = saved;
            expect_op(")");
            /* A parenthesized expression keeps its location; what encloses it spans the parentheses. */
            Tok *close = prev_tok();
            e->pl1 = t->line; e->pc1 = t->col; e->pl2 = close->eline; e->pc2 = close->ecol;
            return e;
        }
        if (is_op(t, "@")) {
            next();
            return mk_expr(X_AT, t);
        }
        if (is_op(t, "<<")) {
            next();
            int saved = P->limit;
            P->limit = 0;
            Expr **a = NULL;
            int n = 0;
            if (!is_op(peek(), ">>") && !is_op(peek(), ">>_"))
                do PUSH(a, n, parse_expr()); while (is_op(peek(), ",") && next());
            P->limit = saved;
            if (is_op(peek(), ">>_")) {
                next();
                e = mk_expr(X_ANGLEACT, t);
                Expr *b[2] = {n == 1 ? a[0] : NULL, parse_subscript()};
                if (n != 1) syntax_error("<<A>>_v takes one action");
                e->n = 2;
                e->args = args_array(b, 2);
                finish(e);
                return e;
            }
            expect_op(">>");
            e = mk_expr(X_TUPLE, t);
            e->n = n;
            e->args = a;
            finish(e);
            return e;
        }
        if (is_op(t, "{")) {
            next();
            int saved = P->limit;
            P->limit = 0;
            Expr **a = NULL;
            int n = 0;
            if (is_op(peek(), "}")) {
                next();
                P->limit = saved;
                e = mk_expr(X_SETENUM, t);
                finish(e);
                return e;
            }
            if (binder_ahead()) {
                e = mk_expr(X_FILTER, t);
                int start = P->pos;
                Bound b;
                memset(&b, 0, sizeof b);
                Expr tmp;
                memset(&tmp, 0, sizeof tmp);
                parse_bounds(&tmp, 0);
                if (tmp.nb == 1 && is_op(peek(), ":") && (tmp.bounds[0].n == 1 || tmp.bounds[0].tuple)) {
                    next();
                    e->bounds = tmp.bounds;
                    e->nb = 1;
                    e->n = 1;
                    e->args = NEWN(Expr *, 1);
                    e->args[0] = parse_expr();
                    expect_op("}");
                    P->limit = saved;
                    finish(e);
                    return e;
                }
                P->pos = start;
            }
            Expr *first = parse_expr();
            if (is_op(peek(), ":")) {
                next();
                e = mk_expr(X_MAP, t);
                parse_bounds(e, 0);
                e->n = 1;
                e->args = NEWN(Expr *, 1);
                e->args[0] = first;
                expect_op("}");
                P->limit = saved;
                finish(e);
                return e;
            }
            PUSH(a, n, first);
            while (is_op(peek(), ",")) {
                next();
                PUSH(a, n, parse_expr());
            }
            expect_op("}");
            P->limit = saved;
            e = mk_expr(X_SETENUM, t);
            e->n = n;
            e->args = a;
            finish(e);
            return e;
        }
        if (is_op(t, "[")) {
            next();
            int saved = P->limit;
            P->limit = 0;
            if (peek()->kind == T_IDENT && (is_op(peek_at(1), "|->") || is_op(peek_at(1), ":"))) {
                int set = is_op(peek_at(1), ":");
                e = mk_expr(set ? X_RCDSET : X_RECORD, t);
                do {
                    Tok *field = peek();
                    int name = expect_ident();
                    for (int i = 0; i < e->nseg; i++)
                        if (e->segs[i].name == name) semantic_error(mk_expr(X_NAME, field), "Non-unique fields in constructor.");
                    PUSH(e->segs, e->nseg, ((Seg){.name = name}));
                    expect_op(set ? ":" : "|->");
                    PUSH(e->args, e->n, parse_expr());
                } while (is_op(peek(), ",") && next());
                expect_op("]");
                P->limit = saved;
                finish(e);
                return e;
            }
            if (binder_ahead()) {
                e = mk_expr(X_FCN, t);
                parse_bounds(e, 0);
                expect_op("|->");
                e->n = 1;
                e->args = NEWN(Expr *, 1);
                e->args[0] = parse_expr();
                expect_op("]");
                P->limit = saved;
                finish(e);
                return e;
            }
            Expr *first = parse_expr();
            if (is_op(peek(), "->")) {
                next();
                e = mk_expr(X_FCNSET, t);
                Expr *a[2] = {first, parse_expr()};
                expect_op("]");
                P->limit = saved;
                e->n = 2;
                e->args = args_array(a, 2);
                finish(e);
                return e;
            }
            if (is_kw(peek(), "EXCEPT")) {
                next();
                e = mk_expr(X_EXCEPT, t);
                PUSH(e->args, e->n, first);
                int n = 0;
                do {
                    Path path = {.line = peek()->line, .col = peek()->col};
                    expect_op("!");
                    Expr *keys[32];
                    int fields[32];
                    int k = 0;
                    while (k < 32) {
                        if (is_op(peek(), ".")) {
                            next();
                            fields[k] = expect_field();
                            keys[k++] = NULL;
                        } else if (is_op(peek(), "[")) {
                            Tok *open = peek();
                            next();
                            Expr *ks[32];
                            int nk = 0;
                            do ks[nk++] = parse_expr(); while (is_op(peek(), ",") && next() && nk < 32);
                            expect_op("]");
                            if (nk == 1) keys[k] = ks[0];
                            else {
                                Expr *tup = mk_expr(X_TUPLE, open);
                                tup->n = nk;
                                tup->args = args_array(ks, nk);
                                finish(tup);
                                keys[k] = tup;
                            }
                            fields[k++] = -1;
                        } else break;
                    }
                    if (!k) syntax_error("expected ! path");
                    expect_op("=");
                    path.n = k;
                    path.keys = args_array(keys, k);
                    path.fields = NEWN(int, k);
                    memcpy(path.fields, fields, sizeof(int) * (size_t)k);
                    PUSH(e->paths, n, path);
                    PUSH(e->args, e->n, parse_expr());
                } while (is_op(peek(), ",") && next());
                expect_op("]");
                P->limit = saved;
                finish(e);
                return e;
            }
            if (is_op(peek(), "]_")) {
                next();
                P->limit = saved;
                e = mk_expr(X_SQACT, t);
                Expr *a[2] = {first, parse_subscript()};
                e->n = 2;
                e->args = args_array(a, 2);
                finish(e);
                return e;
            }
            syntax_error("expected ]_, ->, or EXCEPT");
        }
        if (is_junction(t)) return parse_junction(t);
        break;
    default:
        break;
    }
    syntax_error("unexpected token");
}

/* A primary followed by applications f[a], fields r.f and postfix
   operators x', in any order: x'[1] is (x')[1]. */
static Expr *parse_postfix(void) {
    Expr *e = parse_primary();
    for (;;) {
        Tok *t = peek();
        const OpInfo *post = op_info(t, POSTFIX);
        if (post) {
            next();
            Expr *p = mk_op(t, intern(post->name), 1, &e);
            span_from(p, e);
            e = p;
        } else if (is_op(t, "[")) {
            Tok *start = t;
            next();
            int saved = P->limit;
            P->limit = 0;
            Expr *app = mk_expr(X_APPLY, start);
            PUSH(app->args, app->n, e);
            do PUSH(app->args, app->n, parse_expr()); while (is_op(peek(), ",") && next());
            expect_op("]");
            P->limit = saved;
            span_from(app, e);
            finish(app);
            e = app;
        } else if (is_op(t, ".") && is_field(peek_at(1))) {
            next();
            Expr *d = mk_expr(X_DOT, t);
            span_from(d, e);
            d->op = expect_field();
            d->n = 1;
            d->args = NEWN(Expr *, 1);
            d->args[0] = e;
            finish(d);
            e = d;
        } else {
            return e;
        }
    }
}

/* An operand; *prefix is its prefix operator, if it is an application of one. */
/* lab :: or lab(x, y) :: ahead. */
static int label_ahead(void) {
    if (peek()->kind != T_IDENT) return 0;
    int k = 1;
    if (is_op(peek_at(1), "(")) {
        do if (peek_at(++k)->kind != T_IDENT) return 0;
        while (is_op(peek_at(++k), ","));
        if (!is_op(peek_at(k++), ")")) return 0;
    }
    return is_op(peek_at(k), "::");
}

static Expr *parse_unary(const OpInfo **prefix) {
    Tok *t = peek();
    *prefix = NULL;
    if (label_ahead()) {
        Label *l = NEW(Label);
        l->name = next()->id;
        int names[32];
        if (is_op(peek(), "(")) {
            next();
            do names[l->n++] = expect_ident(); while (is_op(peek(), ",") && next() && l->n < 32);
            expect_op(")");
        }
        l->names = NEWN(int, l->n);
        memcpy(l->names, names, sizeof(int) * (size_t)l->n);
        expect_op("::");
        Expr *e = parse_expr();
        e->label = l;
        return e;
    }
    const OpInfo *info = op_info(t, PREFIX), *named = op_info(t, INFIX);
    if (!named) named = op_info(t, POSTFIX);
    if (!info && named && !is_junction(t) && is_op(peek_at(1), "(")) {
        /* An infix or postfix operator applied to its arguments: +(a, b), ^+(a). */
        next();
        Tok *open = peek();
        int n, want = op_info(t, INFIX) ? 2 : 1;
        Expr **args = parse_args(&n);
        if (n != want) {
            Expr *where = mk_op(open, 0, 0, NULL);
            finish(where);
            semantic_error(where, "The operator %s requires %d arguments.", named->name, want);
        }
        Expr *e = mk_op(t, intern(named->name), n, args);
        finish(e);
        return e;
    }
    if (info) {
        next();
        *prefix = info;
        Expr *arg = parse_binary(info->hi);
        Expr *e = mk_op(t, intern(info->name), 1, &arg);
        span_to(e, arg);
        return e;
    }
    return parse_postfix();
}

/* An expression whose operators all bind tighter than `bound` (§6.1). */
static Expr *parse_binary(int bound) {
    const OpInfo *last;   /* the operator before the next one, as on SANY's stack */
    Expr *left = parse_unary(&last);
    for (;;) {
        Tok *t = peek();
        const OpInfo *post = op_info(t, POSTFIX);
        if (post && post->lo > bound) {
            next();
            Expr *e = mk_op(t, intern(post->name), 1, &left);
            span_from(e, left);
            left = e;
            continue;
        }
        const OpInfo *info = op_info(t, INFIX);
        if (!info) break;
        /* Operators whose ranges overlap conflict, but for a left-associative one repeated. */
        if (last && !(info->hi < last->lo) && !(info == last && info->left))
            parse_error(P->file, t->line, t->col, "Precedence conflict between ops %s and %s", last->name, info->name);
        if (info->lo <= bound) break;
        next();
        if (strcmp(info->name, "\\X") == 0) {
            /* a \X b \X c is a product of three sets. */
            Expr *a[32];
            int n = 0;
            a[n++] = left;
            a[n++] = parse_binary(info->hi);
            while (is_op(peek(), "\\X") || is_op(peek(), "\\times")) {
                next();
                if (n == 32) syntax_error("product too long");
                a[n++] = parse_binary(info->hi);
            }
            Expr *e = mk_op(t, intern("\\X"), n, a);
            span_from(e, left);
            span_to(e, a[n - 1]);
            left = e;
            last = info;
            continue;
        }
        /* SANY rejects a prefix minus right of an operator binding tighter: 2 * -1. */
        if (info->lo > 12 && is_op(peek(), "-"))
            parse_error(P->file, t->line, t->col, "Precedence conflict between ops %s and -.", info->name);
        Expr *right = parse_binary(info->hi);
        Expr *pair[2] = {left, right};
        Expr *e = mk_op(t, intern(info->name), 2, pair);
        span_from(e, left);
        span_to(e, right);
        left = e;
        last = info;
    }
    return left;
}

static Expr *parse_expr(void) { return parse_binary(0); }

static Def *new_def(int kind, int name, Tok *where) {
    Def *d = NEW(Def);
    d->kind = kind;
    d->name = name;
    d->mod = P->mod;
    d->var_index = -1;
    d->level = -1;
    if (where) {
        Expr *w = mk_expr(X_BOOL, where);
        d->where = w;
    }
    return d;
}

/* An operator parameter or constant: x, F(_, _), _ + _, -. _, or _ ^+. */
static Def *parse_param(int kind) {
    Tok *t = peek();
    if (is_op(t, "_")) {
        next();
        Tok *op = peek();
        if (op->kind != T_OP && op->kind != T_KW) syntax_error("expected an operator");
        next();
        Def *d = new_def(kind, canonical(op->id), op);
        if (is_op(peek(), "_")) { next(); d->arity = 2; }
        else d->arity = 1;
        return d;
    }
    if ((t->kind == T_OP || t->kind == T_KW) && is_op(peek_at(1), "_")) {
        next();
        next();
        int name = canonical(t->id);
        if (strcmp(S(name), "-") == 0) name = intern("-.");
        Def *d = new_def(kind, name, t);
        d->arity = 1;
        return d;
    }
    Def *d = new_def(kind, expect_ident(), t);
    if (is_op(peek(), "(")) {
        next();
        do { expect_op("_"); d->arity++; } while (is_op(peek(), ",") && next());
        expect_op(")");
    }
    return d;
}

static Def *parse_instance(Tok *start, int name);

/* Name == e, Name(p) == e, f[x \in S] == e, a + b == e, -. a == e, a ^+ == e. */
static Def *parse_definition(int local) {
    Tok *t = peek();
    Def *d;
    Def *params[64];
    int np = 0;
    if (t->kind == T_IDENT && is_op(peek_at(1), "==")) {
        d = new_def(D_OP, t->id, t);
        next();
    } else if (t->kind == T_IDENT && is_op(peek_at(1), "(")) {
        d = new_def(D_OP, t->id, t);
        next();
        next();
        do {
            if (np == 64) syntax_error("too many parameters");
            params[np++] = parse_param(D_PARAM);
        } while (is_op(peek(), ",") && next());
        expect_op(")");
    } else if (t->kind == T_IDENT && is_op(peek_at(1), "[")) {
        d = new_def(D_FCN, t->id, t);
        next();
        next();
        Expr tmp;
        memset(&tmp, 0, sizeof tmp);
        parse_bounds(&tmp, 0);
        d->bounds = tmp.bounds;
        d->nb = tmp.nb;
        expect_op("]");
    } else if (t->kind == T_IDENT && op_info(peek_at(1), INFIX) &&
               peek_at(2)->kind == T_IDENT && is_op(peek_at(3), "==")) {
        Tok *op = peek_at(1);
        d = new_def(D_OP, canonical(op->id), op);
        params[np++] = new_def(D_PARAM, t->id, t);
        next();
        next();
        params[np++] = new_def(D_PARAM, peek()->id, peek());
        next();
    } else if (op_info(t, PREFIX) && peek_at(1)->kind == T_IDENT && is_op(peek_at(2), "==")) {
        int name = canonical(t->id);
        if (strcmp(S(name), "-") == 0) name = intern("-.");
        d = new_def(D_OP, name, t);
        next();
        params[np++] = new_def(D_PARAM, peek()->id, peek());
        next();
    } else if (t->kind == T_IDENT && op_info(peek_at(1), POSTFIX) && is_op(peek_at(2), "==")) {
        Tok *op = peek_at(1);
        d = new_def(D_OP, canonical(op->id), op);
        params[np++] = new_def(D_PARAM, t->id, t);
        next();
        next();
    } else {
        syntax_error("expected a definition");
    }
    expect_op("==");
    d->local = local;
    d->nparams = np;
    d->params = NEWN(Def *, np);
    memcpy(d->params, params, sizeof(Def *) * (size_t)np);
    if (is_kw(peek(), "INSTANCE")) {
        Tok *start = peek();
        next();
        Def *inst = parse_instance(start, d->name);
        inst->where->l1 = d->where->l1;   /* the statement, from the name */
        inst->where->c1 = d->where->c1;
        inst->nparams = np;
        inst->params = d->params;
        inst->local = local;
        return inst;
    }
    d->body = parse_expr();
    if (d->kind == D_FCN) finish(d->where);   /* TLC's function constructor: the definition */
    return d;
}

/* INSTANCE M WITH a <- e, ...: the substitutions are completed by the
   resolver once M is read. */
static Def *parse_instance(Tok *start, int name) {
    Def *d = new_def(D_INST, name, start);
    d->target = (Module *)(intptr_t)expect_ident();   /* the module's name until resolved */
    int n = 0;
    if (is_kw(peek(), "WITH")) {
        next();
        do {
            Tok *t = peek();
            int pname;
            if (t->kind == T_IDENT) pname = t->id;
            else if (t->kind == T_OP || t->kind == T_KW) pname = canonical(t->id);
            else syntax_error("expected a parameter");
            next();
            expect_op("<-");
            Def *p = NEW(Def);
            p->name = pname;
            PUSH(d->subst_decl, n, p);
            Tok *at = peek();
            if ((at->kind == T_OP || at->kind == T_KW) && (is_op(peek_at(1), ",") || peek_at(1)->line != at->line ||
                                                             peek_at(1)->kind == T_SEP || peek_at(1)->kind == T_END)) {
                Expr *r = mk_expr(X_OPREF, at);
                r->op = canonical(at->id);
                next();
                PUSH(d->subst_expr, d->nsubst, r);
            } else {
                PUSH(d->subst_expr, d->nsubst, is_kw(at, "LAMBDA") ? parse_lambda() : parse_expr());
            }
        } while (is_op(peek(), ",") && next());
    }
    finish(d->where);
    return d;
}

static void add_unit(Module *m, int kind, Def *d, Expr *e, Module *nested, int local) {
    if (m->nunits == m->capunits) {
        m->capunits = m->capunits ? m->capunits * 2 : 64;
        m->units = xrealloc(m->units, sizeof(Unit) * (size_t)m->capunits);
    }
    Unit *u = &m->units[m->nunits++];
    u->kind = kind; u->def = d; u->expr = e; u->module = nested; u->local = local;
}

/* A name SANY records where it appears: a RECURSIVE operator's, a theorem's or an assumption's. */
static void add_name(Module *m, Def *d) { add_unit(m, 4, d, NULL, NULL, 0); }

static Def *thm_name(int name) {
    Def *d = NEW(Def);
    d->kind = D_THM;
    d->name = name;
    return d;
}

static int at_unit_start(void) {
    Tok *t = &P->t[P->pos];
    return t->kind == T_EOF || t->kind == T_END || t->kind == T_SEP || t->col == 1;
}

/* The facts of a HIDE: names of theorems, assumptions, or steps, which
   are the only expressions it may hide. */
static void hide_facts(void) {
    while (!at_unit_start() && !is_kw(peek(), "DEF") && !is_kw(peek(), "DEFS")) {
        Tok *t = next();
        if (is_kw(t, "MODULE")) next();
        else if (t->kind != T_IDENT && t->kind != T_STEP)
            semantic_error(mk_expr(X_NAME, t), "The only expression allowed as a fact in a HIDE is \nthe name of a theorem, assumption, or step.");
        while (is_op(peek(), "!") && !at_unit_start()) { next(); next(); }
        if (!is_op(peek(), ",")) return;
        next();
    }
}

/* A proof, or the rest of USE, HIDE, or BY: up to the next unit. */
static void skip_proof(void) {
    while (!at_unit_start())
        if (is_kw(next(), "HIDE")) hide_facts();
}

/* A module: ---- MODULE Name ---- units ====. */
static Module *parse_module(Module *outer) {
    Module *m = NEW(Module);
    m->outer = outer;
    Module *saved_mod = P->mod;
    P->mod = m;
    if (peek()->kind != T_SEP) syntax_error("expected ---- MODULE");
    next();
    expect_kw("MODULE");
    m->name = expect_ident();
    if (peek()->kind != T_SEP) syntax_error("expected ----");
    next();
    if (is_kw(peek(), "EXTENDS")) {
        next();
        do PUSH(m->extends, m->nextends, expect_ident()); while (is_op(peek(), ",") && next());
    }
    for (;;) {
        Tok *t = peek();
        unit_section = 0;
        if (t->kind == T_END) { next(); break; }
        if (t->kind == T_EOF) syntax_error("missing ==== at the end of the module");
        if (t->kind == T_SEP) {
            if (is_kw(peek_at(1), "MODULE")) {
                Module *inner = parse_module(m);
                add_unit(m, 3, NULL, NULL, inner, 0);
            } else {
                next();
            }
            continue;
        }
        int local = 0;
        Tok *first = t;   /* a statement begins at its LOCAL */
        if (is_kw(t, "LOCAL")) { local = 1; next(); t = peek(); }
        if (is_kw(t, "CONSTANT") || is_kw(t, "CONSTANTS") || is_kw(t, "VARIABLE") || is_kw(t, "VARIABLES")) {
            int var = is_kw(t, "VARIABLE") || is_kw(t, "VARIABLES");
            next();
            do {
                Def *d = parse_param(var ? D_VAR : D_CONST);
                add_unit(m, 0, d, NULL, NULL, 0);
            } while (is_op(peek(), ",") && next());
            continue;
        }
        if (is_kw(t, "ASSUME") || is_kw(t, "ASSUMPTION") || is_kw(t, "AXIOM")) {
            unit_section = 1;
            next();
            Def *d = NULL;
            if (peek()->kind == T_IDENT && is_op(peek_at(1), "==")) { d = thm_name(next()->id); next(); }
            Expr *e = parse_expr();
            if (d) { d->body = e; add_name(m, d); }
            add_unit(m, 2, d, e, NULL, 0);
            continue;
        }
        if (is_kw(t, "THEOREM") || is_kw(t, "LEMMA") || is_kw(t, "PROPOSITION") || is_kw(t, "COROLLARY")) {
            unit_section = 2;
            next();
            Def *d = NULL;
            if (peek()->kind == T_IDENT && is_op(peek_at(1), "==")) { d = thm_name(next()->id); next(); add_name(m, d); }
            /* A theorem's statement is kept for its subexpressions; ASSUME ... PROVE is not. */
            if (!is_kw(peek(), "ASSUME")) {
                Expr *body = parse_expr();
                if (d) d->body = body;
            }
            skip_proof();
            continue;
        }
        if (is_kw(t, "USE") || is_kw(t, "HIDE") || is_kw(t, "PROOF") || is_kw(t, "BY") || is_kw(t, "OBVIOUS") ||
            is_kw(t, "OMITTED") || t->kind == T_STEP) {
            next();
            if (is_kw(t, "HIDE")) hide_facts();
            skip_proof();
            continue;
        }
        if (is_kw(t, "RECURSIVE")) {
            next();
            do add_name(m, parse_param(D_RECURSIVE)); while (is_op(peek(), ",") && next());
            continue;
        }
        if (is_kw(t, "INSTANCE")) {
            next();
            Def *d = parse_instance(first, 0);
            d->local = local;
            add_unit(m, 1, d, NULL, NULL, local);
            continue;
        }
        Def *d = parse_definition(local);
        add_unit(m, 0, d, NULL, NULL, local);
    }
    P->mod = saved_mod;
    return m;
}

/* ---------------------------------------------------------------- modules */

/* The standard modules of tla2tools, without their comments (§8). Their
   operators are evaluated natively; their identifiers are interned as
   TLC interns them. */
static const struct { const char *name, *text; } standard_modules[] = {
    {"Naturals",
     "---- MODULE Naturals ----\n"
     "Nat       == { }\n"
     "a+b       == {a, b}\n"
     "a-b       == CHOOSE n : b + n = a\n"
     "a*b       == TRUE\n"
     "a^b       == {a, b}\n"
     "a<b       ==  a = b\n"
     "a>b       ==  a = b\n"
     "a \\leq b  ==  a = b\n"
     "a \\geq b  ==  a = b\n"
     "a % b     ==  {a, b}\n"
     "a \\div b  ==  {a, b}\n"
     "a .. b    ==  {a, b}\n"
     "====\n"},
    {"Integers",
     "---- MODULE Integers ----\n"
     "EXTENDS Naturals\n"
     "Int  ==  { }\n"
     "-. a == 0 - a\n"
     "====\n"},
    /* TLC's, which cannot compute with real numbers; its definitions on TLC's lines */
    {"Reals",
     "---- MODULE Reals ----\n\n\n\n\n\n\n\n\n\n\n\n"
     "EXTENDS Integers\n"
     "\n"
     "Real  == \"Reals\"\n"
     "a / b == CHOOSE m \\in Real : m * b = a\n"
     "Infinity == CHOOSE i : (i \\notin Real) /\\ (\\A r \\in Real : i > r)\n"
     "====\n"},
    {"Sequences",
     "---- MODULE Sequences ----\n"
     "LOCAL INSTANCE Naturals\n"
     "Seq(S) == UNION {[1..n -> S] : n \\in Nat}\n"
     "Len(s) == CHOOSE n \\in Nat : DOMAIN s = 1..n\n"
     "s \\o t == [i \\in 1..(Len(s) + Len(t)) |-> IF i \\leq Len(s) THEN s[i]\n"
     "                                                           ELSE t[i-Len(s)]]\n"
     "Append(s, e) == s \\o <<e>>\n"
     "Head(s) == s[1]\n"
     "Tail(s) == CASE s # << >> -> [i \\in 1..(Len(s)-1) |-> s[i+1]]\n"
     "SubSeq(s, m, n) == [i \\in 1..(1+n-m) |-> s[i+m-1]]\n"
     "SelectSeq(s, Test(_)) ==\n"
     "  LET F[i \\in 0..Len(s)] ==\n"
     "        IF i = 0 THEN << >>\n"
     "                 ELSE IF Test(s[i]) THEN Append(F[i-1], s[i])\n"
     "                                    ELSE F[i-1]\n"
     "  IN F[Len(s)]\n"
     "====\n"},
    {"FiniteSets",
     "---- MODULE FiniteSets ----\n"
     "LOCAL INSTANCE Naturals\n"
     "LOCAL INSTANCE Sequences\n"
     "IsFiniteSet(S) ==\n"
     "  \\E seq \\in Seq(S) : \\A s \\in S : \\E n \\in 1..Len(seq) : seq[n] = s\n"
     "Cardinality(S) ==\n"
     "  LET CS[T \\in SUBSET S] == IF T = {} THEN 0\n"
     "                                      ELSE 1 + CS[T \\ {CHOOSE x : x \\in T}]\n"
     "  IN  CS[S]\n"
     "====\n"},
    {"TLC",
     "---- MODULE TLC ----\n"
     "LOCAL INSTANCE Naturals\n"
     "LOCAL INSTANCE Sequences\n"
     "LOCAL INSTANCE FiniteSets\n"
     "Print(out, val) == val\n"
     "PrintT(out) == TRUE\n"
     "Assert(val, out) == IF val = TRUE THEN TRUE\n"
     "                                  ELSE CHOOSE v : TRUE\n"
     "JavaTime == CHOOSE n : n \\in Nat\n"
     "TLCGet(i) == CHOOSE n : TRUE\n"
     "TLCSet(i, v) == TRUE\n"
     "d :> e == [x \\in {d} |-> e]\n"
     "f @@ g == [x \\in (DOMAIN f) \\cup (DOMAIN g) |->\n"
     "            IF x \\in DOMAIN f THEN f[x] ELSE g[x]]\n"
     "Permutations(S) ==\n"
     "   {f \\in [S -> S] : \\A w \\in S : \\E v \\in S : f[v]=w}\n"
     "SortSeq(s, Op(_, _)) ==\n"
     "    LET Perm == CHOOSE p \\in Permutations(1 .. Len(s)) :\n"
     "                  \\A i, j \\in 1..Len(s) :\n"
     "                     (i < j) => Op(s[p[i]], s[p[j]]) \\/ (s[p[i]] = s[p[j]])\n"
     "    IN  [i \\in 1..Len(s) |-> s[Perm[i]]]\n"
     "RandomElement(s) == CHOOSE x \\in s : TRUE\n"
     "Any == CHOOSE x : TRUE\n"
     "ToString(v) == (CHOOSE x \\in [a : v, b : STRING] : TRUE).b\n"
     "TLCEval(v) == v\n"
     "====\n"},
    {"Bags",
     "---- MODULE Bags ----\n"
     "EXTENDS TLC\n"
     "LOCAL INSTANCE Naturals\n"
     "IsABag(B) ==\n"
     "  B \\in [DOMAIN B -> {n \\in Nat : n > 0}]\n"
     "BagToSet(B) == DOMAIN B\n"
     "SetToBag(S) == [e \\in S |-> 1]\n"
     "BagIn(e,B) == e \\in BagToSet(B)\n"
     "EmptyBag == SetToBag({})\n"
     "B1 (+) B2  ==\n"
     "  [e \\in (DOMAIN B1) \\cup (DOMAIN B2) |->\n"
     "      (IF e \\in DOMAIN B1 THEN B1[e] ELSE 0)\n"
     "    + (IF e \\in DOMAIN B2 THEN B2[e] ELSE 0) ]\n"
     "B1 (-) B2  ==\n"
     "  LET B == [e \\in DOMAIN B1 |-> IF e \\in DOMAIN B2 THEN B1[e] - B2[e]\n"
     "                                                   ELSE B1[e]]\n"
     "  IN  [e \\in {d \\in DOMAIN B : B[d] > 0} |-> B[e]]\n"
     "LOCAL Sum(f) ==\n"
     "        LET DSum[S \\in SUBSET DOMAIN f] ==\n"
     "               LET elt == CHOOSE e \\in S : TRUE\n"
     "               IN  IF S = {} THEN 0\n"
     "                             ELSE f[elt] + DSum[S \\ {elt}]\n"
     "        IN  DSum[DOMAIN f]\n"
     "BagUnion(S) ==\n"
     "  [e \\in UNION {BagToSet(B) : B \\in S} |->\n"
     "    Sum( [B \\in S |-> IF BagIn(e, B) THEN B[e] ELSE 0] ) ]\n"
     "B1 \\sqsubseteq B2  ==\n"
     "  /\\ (DOMAIN B1) \\subseteq (DOMAIN B2)\n"
     "  /\\ \\A e \\in DOMAIN B1 : B1[e] \\leq B2[e]\n"
     "SubBag(B) ==\n"
     "  LET RemoveFromDom(x, f) == [y \\in (DOMAIN f) \\ {x} |-> f[y]]\n"
     "      Combine(x, BagSet) ==\n"
     "         BagSet \\cup\n"
     "          {[y \\in (DOMAIN f) \\cup {x} |-> IF y = x THEN i ELSE f[y]] :\n"
     "                f \\in BagSet, i \\in 1..B[x]}\n"
     "      Biggest == LET Range == {B[x] : x \\in DOMAIN B}\n"
     "                 IN  IF Range = {} THEN 0\n"
     "                                   ELSE CHOOSE r \\in Range :\n"
     "                                           \\A s \\in Range : r \\geq s\n"
     "      RSB[BB \\in UNION {[S -> 1..Biggest] : S \\in SUBSET DOMAIN B}] ==\n"
     "        IF BB = << >> THEN {<< >>}\n"
     "                      ELSE LET x == CHOOSE x \\in DOMAIN BB : TRUE\n"
     "                           IN  Combine(x, RSB[RemoveFromDom(x, BB)])\n"
     "  IN RSB[B]\n"
     "BagOfAll(F(_), B) ==\n"
     "  [e \\in {F(d) : d \\in BagToSet(B)} |->\n"
     "     Sum( [d \\in BagToSet(B) |-> IF F(d) = e THEN B[d] ELSE 0] ) ]\n"
     "BagCardinality(B) ==\n"
     "  Sum(B)\n"
     "CopiesIn(e, B) ==\n"
     "  IF BagIn(e, B) THEN B[e] ELSE 0\n"
     "====\n"},
    {"Randomization",
     "---- MODULE Randomization ----\n"
     "LOCAL INSTANCE Naturals\n"
     "LOCAL INSTANCE FiniteSets\n"
     "RandomSubset(k, S) == CHOOSE T \\in SUBSET S : Cardinality(T) = T\n"
     "RandomSetOfSubsets(k, n, S) == CHOOSE T \\in SUBSET SUBSET S :\n"
     "                                              Cardinality(T) \\leq k\n"
     "TestRandomSetOfSubsets(k, n, S) ==\n"
     "              [i \\in 1..5 |-> Cardinality(RandomSetOfSubsets(k, n, S))]\n"
     "====\n"},
    {NULL, NULL}};

static Module **modules;      /* in the order they were read */
static int nmodules;
static Module **finished;     /* each after the modules it uses, as SANY finishes them */
static int nfinished;

static void finish_module(Module *m) {
    finished = xrealloc(finished, sizeof(Module *) * (size_t)(nfinished + 1));
    finished[nfinished++] = m;
}
static char *spec_dir;

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *s = xmalloc((size_t)n + 1);
    if (fread(s, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(s); return NULL; }
    s[n] = 0;
    fclose(f);
    return s;
}

static void collect_nested(Module *m) {
    for (int i = 0; i < m->nunits; i++)
        if (m->units[i].kind == 3) {
            m->nested = xrealloc(m->nested, sizeof(Module *) * (size_t)(m->nnested + 1));
            m->nested[m->nnested++] = m->units[i].module;
            collect_nested(m->units[i].module);
        }
}

static Module *find_module(int name);
static Module *load_module(int name, Module *from);

/* Reads the modules a module refers to, in order of reference. */
static void load_references(Module *m) {
    for (int i = 0; i < m->nextends; i++) load_module(m->extends[i], m);
    for (int i = 0; i < m->nunits; i++) {
        Unit *u = &m->units[i];
        if (u->kind == 1 || (u->kind == 0 && u->def->kind == D_INST))
            load_module((int)(intptr_t)u->def->target, m)->instantiated = 1;
        if (u->kind == 3) load_references(u->module);
    }
}

static Module *find_module(int name) {
    for (int i = 0; i < nmodules; i++) if (modules[i]->name == name) return modules[i];
    return NULL;
}

static Module *parse_text(const char *text, const char *path) {
    int n;
    Tok *toks = lex(text, path, &n);
    Parser p = {toks, 0, n, 0, NULL, path};
    Parser *saved = P;
    P = &p;
    Module *m = parse_module(NULL);
    P = saved;
    m->path = (char *)path;
    m->toks = toks;
    m->ntoks = n;
    collect_nested(m);
    return m;
}

static void check_file_name(Module *m, const char *name) {
    if (strcmp(S(m->name), name) == 0) return;
    printf("File name '%s' does not match the name '%s' of the top level module it contains.\n\n", name, S(m->name));
    printf("Error: Parsing or semantic analysis failed.\n");
    exit(TPP_ERROR_PARSE);
}

static Module *load_module(int name, Module *from) {
    for (Module *o = from; o; o = o->outer)
        for (int i = 0; i < o->nnested; i++) if (o->nested[i]->name == name) return o->nested[i];
    Module *m = find_module(name);
    if (m) return m;
    const char *text = NULL;
    char *path = xmalloc(strlen(spec_dir) + strlen(S(name)) + 8);
    sprintf(path, "%s%s.tla", spec_dir, S(name));
    char *file = read_file(path);
    int standard = 0;
    if (file) text = file;
    else {
        for (int i = 0; standard_modules[i].name; i++)
            if (strcmp(standard_modules[i].name, S(name)) == 0) { text = standard_modules[i].text; standard = 1; }
        sprintf(path, "%s.tla", S(name));
    }
    if (!text) {
        printf("Cannot find source file for module %s imported in module %s.\n\n", S(name), S(from->name));
        printf("Error: Parsing or semantic analysis failed.\n");
        exit(TPP_ERROR_PARSE);
    }
    m = parse_text(text, path);
    m->standard = standard;
    check_file_name(m, S(name));
    modules = xrealloc(modules, sizeof(Module *) * (size_t)(nmodules + 1));
    modules[nmodules++] = m;
    load_references(m);
    finish_module(m);
    return m;
}

/* ---------------------------------------------------------------- resolver */

typedef struct Scope { int name; Def *def; struct Scope *next; } Scope;

static Module *cur_module;   /* the module being resolved */

static void loc_text(Buf *b, Expr *e) {
    if (!e || !e->mod) { buf_puts(b, "Unknown location"); return; }
    buf_printf(b, "line %d, col %d to line %d, col %d of module %s", e->l1, e->c1, e->l2, e->c2, S(e->mod->name));
}

/* An expression as TLC names it after a message: a number as written,
   else its location. */
static void node_text(Buf *b, Expr *e) {
    if (e && e->kind == X_NUM) buf_puts(b, S(e->op));
    else loc_text(b, e);
}

/* A new line naming e, if it has a location. */
static void loc_line(Buf *b, Expr *e) {
    if (!e || !e->mod) return;
    buf_puts(b, "\n");
    node_text(b, e);
}

static _Noreturn void semantic_error(Expr *where, const char *fmt, ...) {
    char msg[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    Buf b = {0};
    loc_text(&b, where);
    printf("Semantic errors:\n\n*** Errors: 1\n\n%s\n\n%s\n\n", b.s, msg);
    printf("Error: Parsing or semantic analysis failed.\n");
    exit(TPP_ERROR_PARSE);
}

static Def *scope_lookup(Scope *sc, int name);
static Sym *module_lookup(Module *m, int name);

/* A new name in scope, which must not hide one (§5.6). */
static Scope *bind(Scope *sc, int name, Def *d) {
    Def *old = scope_lookup(sc, name);
    Sym *sym = old || !cur_module ? NULL : module_lookup(cur_module, name);
    if ((old && old != d) || (sym && sym->def != d))
        semantic_error(d->where, "Multiply-defined symbol '%s': this definition or declaration conflicts with another one.", S(name));
    Scope *s = NEW(Scope);
    s->name = name; s->def = d; s->next = sc;
    return s;
}

static Sym *module_lookup(Module *m, int name) {
    for (Module *o = m; o; o = o->outer) {
        int limit = o == m ? o->nsyms : 0;
        if (o != m) {
            /* an enclosing module's symbols defined before the nested module */
            Module *inner = m;
            while (inner->outer != o) inner = inner->outer;
            limit = inner->nouter_syms;
        }
        for (int i = limit - 1; i >= 0; i--) if (o->syms[i].name == name) return &o->syms[i];
    }
    return NULL;
}

/* A nested module's own declaration or definition of a name its enclosing
   modules already give it: SANY warns of a declaration repeated, of the
   same kind and arity, and rejects any other. */
static Module *file_module(Module *m) {
    while (m->outer) m = m->outer;
    return m;
}

/* A location as SANY gives it, in the module of its file. */
static void file_loc_text(Buf *b, Expr *e) {
    buf_printf(b, "line %d, col %d to line %d, col %d of module %s", e->l1, e->c1, e->l2, e->c2, S(file_module(e->mod)->name));
}

static void check_outer(Module *m, Def *d) {
    Sym *s = m->outer ? module_lookup(m, d->name) : NULL;
    if (!s || (m->nsyms && s >= m->syms && s < m->syms + m->nsyms)) return;   /* its own: add_sym's */
    Def *o = s->def;
    Buf here = {0}, there = {0};
    file_loc_text(&here, d->where);
    file_loc_text(&there, o->where);
    if ((d->kind != D_CONST && d->kind != D_VAR) || o->kind != d->kind || o->arity != d->arity)
        semantic_error(d->where, "Multiple declarations or definitions for symbol %s.\nThis duplicates the one at %s.", S(d->name), there.s);
    Module *file = file_module(m);
    buf_printf(&file->warnings, "%s\n\nMultiple declarations or definitions for symbol %s.  \nThis duplicates the one at %s.\n\n\n",
               here.s, S(d->name), there.s);
    file->nwarnings++;
}

/* SANY's warnings so far, printed as each file's module is processed;
   those of its global context, of definitions two extended modules give
   a name, before them. */
static Buf warnings, context_warnings;
static int nwarnings, ncontext_warnings;

static void print_warnings(Module *m) {
    buf_add(&warnings, m->warnings.s ? m->warnings.s : "", m->warnings.n);
    nwarnings += m->nwarnings;
    if (ncontext_warnings) printf("Semantic errors in global context:\n\n\n*** Warnings: %d\n\n%s", ncontext_warnings, context_warnings.s);
    if (nwarnings) printf("Semantic errors:\n\n*** Warnings: %d\n\n%s\n", nwarnings, warnings.s);
}

static Sym *own_sym(Module *m, int name) {
    for (int i = 0; i < m->nsyms; i++) if (m->syms[i].name == name) return &m->syms[i];
    return NULL;
}

/* A definition's location as SANY gives it: from its name to the end of its body. */
static Expr def_span(Def *d) {
    Expr e = *d->where;
    Expr *b = d->body;
    if (b) { e.l2 = b->pl2 ? b->pl2 : b->l2; e.c2 = b->pl2 ? b->pc2 : b->c2; }
    return e;
}

/* Whether the definition d, which m extends or, with instance, instantiates,
   gives way to another of its name m has: as SANY, with a warning, keeping
   the first. A declaration conflicts. The warning is at d, or at a LOCAL
   INSTANCE. */
static int inherited_conflict(Module *m, Def *d, Def *instance) {
    Sym *old = own_sym(m, d->name);
    if (!old || old->def == d || d->kind == D_CONST || d->kind == D_VAR || old->def->kind == D_CONST || old->def->kind == D_VAR)
        return 0;
    Buf here = {0}, there = {0};
    Expr at = instance && instance->local ? *instance->where : def_span(d), span = def_span(old->def);
    file_loc_text(&here, &at);
    file_loc_text(&there, &span);
    if (instance) {
        Module *file = file_module(m);
        buf_printf(&file->warnings, "%s\n\nMultiple declarations or definitions for symbol %s.  \nThis duplicates the one at %s.\n\n\n",
                   here.s, S(d->name), there.s);
        file->nwarnings++;
    } else {
        buf_printf(&context_warnings, "%s\n\nWarning: the definition of '%s' conflicts with \nits definition at %s.\n\n\n",
                   here.s, S(d->name), there.s);
        ncontext_warnings++;
    }
    free(here.s);
    free(there.s);
    return 1;
}

static void add_sym(Module *m, int name, Def *def, Def **path, int npath, int local, Expr *where) {
    for (int i = 0; i < m->nsyms; i++)
        if (m->syms[i].name == name) {
            if (m->syms[i].def == def) return;
            if (def->kind != D_RECURSIVE && m->syms[i].def->kind != D_RECURSIVE && npath == 0 && m->syms[i].npath == 0)
                semantic_error(where, "Multiply-defined symbol '%s': this definition or declaration conflicts with another one.", S(name));
            m->syms[i].def = def; m->syms[i].path = path; m->syms[i].npath = npath; m->syms[i].local = local;
            return;
        }
    if (m->nsyms == m->capsyms) {
        m->capsyms = m->capsyms ? m->capsyms * 2 : 64;
        m->syms = xrealloc(m->syms, sizeof(Sym) * (size_t)m->capsyms);
    }
    Sym *s = &m->syms[m->nsyms++];
    s->name = name; s->def = def; s->path = path; s->npath = npath; s->local = local;
}

static void add_decl(Module *m, Def *d) {
    for (int i = 0; i < m->ndecls; i++) if (m->decls[i] == d) return;
    if (m->ndecls == m->capdecls) {
        m->capdecls = m->capdecls ? m->capdecls * 2 : 16;
        m->decls = xrealloc(m->decls, sizeof(Def *) * (size_t)m->capdecls);
    }
    m->decls[m->ndecls++] = d;
}

static void resolve_expr(Expr *e, Scope *sc);
static void resolve_def(Def *d, Scope *sc);
static void resolve_module(Module *m);
static void check_levels(Expr *e, int max);
static void check_def_levels(Def *d);

static Def *scope_lookup(Scope *sc, int name) {
    for (; sc; sc = sc->next) if (sc->name == name) return sc->def;
    return NULL;
}

static void check_arity(Expr *e, Def *d, int n) {
    int want = d->kind == D_OP ? d->nparams : d->kind == D_PARAM || d->kind == D_CONST ? d->arity : 0;
    if (d->kind == D_INST) want = d->nparams;
    if (n != want) {
        if (want == 0) semantic_error(e, "The operator %s requires 0 arguments.", S(d->name));
        semantic_error(e, "The operator %s requires %d arguments.", S(d->name), want);
    }
}

static void resolve_args(Expr **args, int n, Scope *sc) {
    for (int i = 0; i < n; i++) if (args[i]) resolve_expr(args[i], sc);
}

/* An operator symbol passed as an argument: a Def to call. Core
   operators are wrapped in a definition that applies them. */
static Def *operator_def(Expr *at, int op, Scope *sc) {
    Def *d = scope_lookup(sc, op);
    if (d) return d;
    Sym *s = module_lookup(cur_module, op);
    if (s && !s->npath) return s->def;
    /* A built-in operator: an infix one takes two arguments, others one. */
    int n = 1;
    for (int i = 0; op_table[i].name; i++) if (op_table[i].fix == INFIX && strcmp(op_table[i].name, S(op)) == 0) n = 2;
    Def *w = NEW(Def);
    w->kind = D_OP;
    w->name = op;
    w->nparams = n;
    w->params = NEWN(Def *, n);
    w->level = -1;
    w->var_index = -1;
    w->mod = cur_module;
    Expr *body = NEW(Expr);
    *body = *at;
    body->kind = X_OP;
    body->op = op;
    body->n = n;
    body->args = NEWN(Expr *, n);
    Scope *inner = sc;
    for (int i = 0; i < n; i++) {
        w->params[i] = NEW(Def);
        w->params[i]->kind = D_PARAM;
        w->params[i]->name = intern(i ? "#b" : "#a");
        inner = bind(inner, w->params[i]->name, w->params[i]);
        Expr *ref = NEW(Expr);
        *ref = *at;
        ref->kind = X_NAME;
        ref->nseg = 1;
        ref->segs = NEW(Seg);
        ref->segs->name = w->params[i]->name;
        ref->op = w->params[i]->name;
        body->args[i] = ref;
    }
    resolve_expr(body, inner);
    w->body = body;
    return w;
}

static int expr_level(Expr *e);

/* TLA+ has no raw actions in temporal formulas (§6.10). */
static void check_temporal(Expr *e) {
    switch (e->kind) {
    case K_ALWAYS:
        if (expr_level(e->args[0]) == 2 && e->args[0]->kind != X_SQACT)
            semantic_error(e, "[] followed by action not of form [A]_v.");
        return;
    case K_EVENTUALLY:
        if (expr_level(e->args[0]) == 2 && e->args[0]->kind != X_ANGLEACT)
            semantic_error(e, "<> followed by action not of form <<A>>_v.");
        return;
    case K_LEADSTO: case K_WHILE:
        if (expr_level(e->args[0]) == 2 || expr_level(e->args[1]) == 2)
            semantic_error(e, "Action used where only temporal formula or state predicate allowed.");
        return;
    case X_FORALL: case X_EXISTS:
        if (expr_level(e->args[0]) == 3)
            for (int i = 0; i < e->nb; i++)
                if (e->bounds[i].set && expr_level(e->bounds[i].set) == 2)
                    semantic_error(e->bounds[i].set, "Action-level bound of quantified temporal formula.");
        return;
    }
}

static void resolve_bounds(Expr *e, Scope **sc) {
    Scope *outer = *sc;
    for (int i = 0; i < e->nb; i++) {
        Bound *b = &e->bounds[i];
        if (b->set) resolve_expr(b->set, outer);
        b->defs = NEWN(Def *, b->n);
        for (int j = 0; j < b->n; j++) {
            Def *d = NEW(Def);
            d->kind = D_BOUND;
            d->name = b->names[j];
            d->var_index = -1;
            b->defs[j] = d;
            *sc = bind(*sc, d->name, d);
        }
    }
}

static const struct { const char *name; int kind; } core_ops[] = {
    {"/\\", K_AND}, {"\\/", K_OR}, {"~", K_NOT}, {"=>", K_IMPLIES}, {"<=>", K_EQUIV}, {"=", K_EQ},
    {"/=", K_NEQ}, {"\\in", K_IN}, {"\\notin", K_NOTIN}, {"\\cup", K_CUP}, {"\\cap", K_CAP},
    {"\\", K_SETMINUS}, {"\\subseteq", K_SUBSETEQ}, {"SUBSET", K_SUBSET}, {"UNION", K_UNION},
    {"DOMAIN", K_DOMAIN}, {"'", K_PRIME}, {"UNCHANGED", K_UNCHANGED}, {"ENABLED", K_ENABLED},
    {"[]", K_ALWAYS}, {"<>", K_EVENTUALLY}, {"~>", K_LEADSTO}, {"\\X", K_TIMES}, {"\\cdot", K_CDOT},
    {"-+->", K_WHILE}, {"BOOLEAN", K_BOOLEAN}, {"STRING", K_STRINGSET}, {NULL, 0}};

/* The expression labeled `name` in x, but not in another label in it or
   in a LET definition, which have labels of their own (§5.7). */
static Expr *find_label(Expr *x, int name, int own) {
    if (!x) return NULL;
    if (x->label && !own) return x->label->name == name ? x : NULL;
    if (x->kind == X_LET) return find_label(x->args[0], name, 0);
    Expr *l = NULL;
    for (int i = 0; i < x->n && !l; i++) l = find_label(x->args[i], name, 0);
    for (int i = 0; i < x->nb && !l; i++) l = find_label(x->bounds[i].set, name, 0);
    return l;
}

/* The LET expressions on the way from x to y, outermost first, after n
   others; -1 if y is not in x. */
static int lets_to(Expr *x, Expr *y, Expr **lets, int n) {
    if (!x) return -1;
    if (x == y) return n;
    int r = -1;
    if (x->kind == X_LET && n < 64) {
        lets[n++] = x;
        for (int i = 0; i < x->ndefs && r < 0; i++) r = lets_to(x->defs[i]->body, y, lets, n);
    }
    for (int i = 0; i < x->n && r < 0; i++) r = lets_to(x->args[i], y, lets, n);
    for (int i = 0; i < x->nb && r < 0; i++) r = lets_to(x->bounds[i].set, y, lets, n);
    for (int i = 0; x->kind == X_EXCEPT && i < x->n - 1 && r < 0; i++)
        for (int k = 0; k < x->paths[i].n && r < 0; k++) r = lets_to(x->paths[i].keys[k], y, lets, n);
    return r;
}

/* Which of n operands selector sel picks, from 1; 0 for none. */
static int operand(int sel, int n) {
    if (sel > 0) return sel <= n ? sel : 0;
    if (sel == SEL_LL) return n > 0;
    if (sel == SEL_GG) return n == 2 ? 2 : 0;
    return 0;
}

/* The parameters met selecting a subexpression, and their arguments. */
typedef struct { Def *params[64]; Expr *args[64]; int np, nargs, missing; } Selection;

/* The parameters defs, with the arguments seg gives them, if any. */
static void take(Selection *sel, Expr *e, Def **defs, int n, Seg *seg) {
    if (seg->n && seg->n != n) semantic_error(e, "The operator %s requires %d arguments.", S(e->op), n);
    if (sel->np + n > 64) semantic_error(e, "Too many parameters.");
    for (int k = 0; k < n; k++) {
        sel->params[sel->np++] = defs[k];
        if (seg->n) sel->args[sel->nargs++] = seg->args[k];
    }
    sel->missing |= n && !seg->n;
}

/* d(a)!lab(b)!2!(c)...: the subexpression that segments i.. of e name, as
   an operator of every parameter met on the way: d's, the labels', and the
   identifiers !(c) or !@ bind. Its arguments, if given, are in *args. */
static Def *subexpression(Expr *e, Def *d, int i, Expr ***args, int *nargs) {
    if (d->kind != D_OP || d->local == 2 || !d->body)
        semantic_error(e, "Cannot take subexpression of `%s'.", S(d->name));
    Selection sel = {0};
    take(&sel, e, d->params, d->nparams, &e->segs[i]);
    Expr *cur = d->body;
    int names = 1, own = 0;   /* a name next is a label, else a LET definition */
    for (int k = i + 1; k < e->nseg; k++) {
        Seg *s = &e->segs[k];
        if (s->sel == 0 && names) {
            cur = find_label(cur, s->name, own);
            if (!cur) semantic_error(e, "Unknown label `%s'.", S(s->name));
            take(&sel, e, cur->label->defs, cur->label->n, s);
            own = 1;
        } else if (s->sel == 0) {
            Def *x = NULL;
            for (int j = 0; cur->kind == X_LET && j < cur->ndefs && !x; j++)
                if (cur->defs[j]->name == s->name) x = cur->defs[j];
            if (!x || !x->body) semantic_error(e, "Unknown operator: `%s'.", S(s->name));
            take(&sel, e, x->params, x->nparams, s);
            cur = x->body;
            names = 1, own = 0;
        } else if (s->sel == SEL_COLON) {
            names = 0;
        } else {
            int n = 0, first = k;
            if (cur->kind == X_LET) {
                if (operand(s->sel, 1)) n = 1, cur = cur->args[0];
            } else if (cur->nb) {
                if (s->sel == SEL_NULL || s->sel == SEL_AT) {
                    Def *bound[64];
                    int nbound = 0;
                    for (int b = 0; b < cur->nb; b++)
                        for (int j = 0; j < cur->bounds[b].n && nbound < 64; j++) bound[nbound++] = cur->bounds[b].defs[j];
                    take(&sel, e, bound, nbound, s);
                    n = 1, cur = cur->args[0];
                } else if ((n = operand(s->sel, cur->bounds[0].set ? cur->nb : 0))) {
                    cur = cur->bounds[n - 1].set;
                }
            } else if (cur->kind == X_CASE) {   /* an arm, then its guard or value */
                int arm = operand(s->sel, cur->n / 2);
                if (arm && ++k < e->nseg && (n = operand(e->segs[k].sel, 2))) cur = cur->args[2 * arm - 3 + n];
            } else if (cur->kind == X_DOT) {
                if ((n = operand(s->sel, 2)) == 1) cur = cur->args[0];
                else if (n) {
                    Expr *f = NEW(Expr);
                    *f = *cur;
                    f->kind = X_STR, f->n = 0, f->label = NULL;
                    f->num = cur->op;
                    cur = f;
                }
            } else if ((n = operand(s->sel, cur->n)) && (cur->kind != X_EXCEPT || n == 1)) {
                cur = cur->args[n - 1];
            }
            if (!n || !cur) semantic_error(e, "Nonexistent operand specified by selector %d of %s.", first - i, S(d->name));
            names = 0, own = 0;
        }
    }
    if (sel.missing && sel.nargs) semantic_error(e, "The operator %s requires %d arguments.", S(e->op), sel.np);
    /* The LET definitions around the subexpression are in scope in it. */
    Expr *lets[64];
    Expr *body = cur;
    for (int n = lets_to(d->body, cur, lets, 0); n > 0; n--) {
        Expr *let = NEW(Expr);
        *let = *lets[n - 1];
        let->label = NULL;
        let->args = NEWN(Expr *, 1);
        let->args[0] = body;
        body = let;
    }
    Def *f = NEW(Def);
    f->kind = D_OP;
    f->name = d->name;
    f->mod = d->mod;
    f->where = d->where;
    f->level = f->var_index = -1;
    f->nparams = sel.np;
    f->params = NEWN(Def *, sel.np);
    memcpy(f->params, sel.params, sizeof(Def *) * (size_t)sel.np);
    f->body = body;
    *nargs = sel.nargs;
    *args = NEWN(Expr *, sel.nargs);
    memcpy(*args, sel.args, sizeof(Expr *) * (size_t)sel.nargs);
    return f;
}

/* Resolves A!B(x)!C: each segment but the last names an instance, or
   the segments after a definition select a subexpression of it. */
static void resolve_path(Expr *e, Scope *sc) {
    Def *path[32];
    Seg pargs[32];
    int np = 0;
    memset(pargs, 0, sizeof pargs);
    Module *m = cur_module;
    Def *d = NULL;
    Expr **args = NULL;
    int nargs = 0;
    for (int i = 0; i < e->nseg; i++) resolve_args(e->segs[i].args, e->segs[i].n, sc);
    for (int i = 0; i < e->nseg; i++) {
        Seg *s = &e->segs[i];
        Sym *sym = NULL;
        if (i == 0) {
            d = scope_lookup(sc, s->name);
            if (!d) sym = module_lookup(m, s->name);
        } else {
            sym = module_lookup(m, s->name);
            if (sym && sym->local) sym = NULL;
        }
        if (sym) {
            for (int k = 0; k < sym->npath && np < 32; k++) path[np++] = sym->path[k];
            d = sym->def;
        }
        if (!d) semantic_error(e, "Unknown operator: `%s'.", S(s->name));
        if (i + 1 < e->nseg && d->kind == D_INST) {
            check_arity(e, d, s->n);
            pargs[np] = *s;
            path[np++] = d;
            m = d->target;
            d = NULL;
        } else if (i + 1 < e->nseg) {
            d = subexpression(e, d, i, &args, &nargs);
            break;
        } else {
            check_arity(e, d, s->n);
            args = s->args;
            nargs = s->n;
        }
    }
    e->kind = np ? K_INSTCALL : K_CALL;
    e->def = d;
    e->defs = NEWN(Def *, np);
    memcpy(e->defs, path, sizeof(Def *) * (size_t)np);
    e->ndefs = np;
    e->args = args;
    e->n = nargs;
    /* segs now holds each instance's arguments, aligned with defs. */
    e->segs = NEWN(Seg, np);
    memcpy(e->segs, pargs, sizeof(Seg) * (size_t)np);
    e->nseg = np;
}

static void resolve_name(Expr *e, Scope *sc) {
    if (e->nseg > 1) { resolve_path(e, sc); return; }
    Seg *s = &e->segs[0];
    resolve_args(s->args, s->n, sc);
    e->args = s->args;
    e->n = s->n;
    Def *d = scope_lookup(sc, s->name);
    if (d) {
        if (d->kind == D_INST) semantic_error(e, "Operator name %s is incomplete.", S(d->name));
        if (d->kind == D_BOUND || (d->kind == D_PARAM && d->arity == 0 && s->n == 0)) {
            if (s->n) semantic_error(e, "The operator %s requires 0 arguments.", S(d->name));
            e->kind = K_BOUND;
        } else if (d->kind == D_PARAM) {
            if (s->n) check_arity(e, d, s->n);   /* bare, it is an operator argument */
            e->kind = K_PARAMCALL;
        } else {
            if (s->n || !d->nparams) check_arity(e, d, s->n);
            e->kind = K_CALL;
        }
        e->def = d;
        return;
    }
    Sym *sym = module_lookup(cur_module, s->name);
    if (!sym || sym->def->kind == D_RECURSIVE) {   /* located, as SANY, at the name */
        Expr name = *e;
        name.l2 = name.l1;
        name.c2 = name.c1 + (int)strlen(S(s->name)) - 1;
        semantic_error(&name, "Unknown operator: `%s'.", S(s->name));
    }
    d = sym->def;
    if (d->kind == D_INST) semantic_error(e, "Operator name %s is incomplete.", S(d->name));
    if (!(d->kind == D_OP && s->n == 0 && d->nparams > 0)) check_arity(e, d, s->n);
    e->def = d;
    if (d->kind == D_CONST || d->kind == D_VAR) {
        e->kind = d->arity ? K_PARAMCALL : K_DECL;
    } else if (sym->npath) {
        e->kind = K_INSTCALL;
        e->defs = sym->path;
        e->ndefs = sym->npath;
        e->segs = NEWN(Seg, sym->npath);
        e->nseg = sym->npath;
    } else {
        e->kind = K_CALL;
    }
}

/* The function definitions being resolved: one named in its own body is
   recursive (§6.6). */
static Def *open_defs[64];
static int nopen_defs;

static void resolve_expr(Expr *e, Scope *sc) {
    if (e->label && !e->label->defs) {
        Label *l = e->label;
        l->defs = NEWN(Def *, l->n);
        for (int i = 0; i < l->n; i++)
            if (!(l->defs[i] = scope_lookup(sc, l->names[i]))) semantic_error(e, "Unknown operator: `%s'.", S(l->names[i]));
    }
    switch (e->kind) {
    case X_NUM: case X_BOOL: case X_AT:
        return;
    case X_STR:
        e->num = intern_n(e->text, e->len);
        return;
    case X_NAME:
        resolve_name(e, sc);
        for (int i = 0; i < nopen_defs; i++) if (open_defs[i] == e->def) e->def->self = 1;
        return;
    case X_OPREF:
        e->def = operator_def(e, e->op, sc);
        return;
    case X_LAMBDA: {
        Scope *inner = sc;
        for (int i = 0; i < e->def->nparams; i++) inner = bind(inner, e->def->params[i]->name, e->def->params[i]);
        e->def->mod = cur_module;
        resolve_expr(e->def->body, inner);
        return;
    }
    case X_OP: {
        resolve_args(e->args, e->n, sc);
        for (int i = 0; core_ops[i].name; i++)
            if (strcmp(core_ops[i].name, S(e->op)) == 0) { e->kind = (short)core_ops[i].kind; check_temporal(e); return; }
        Def *d = scope_lookup(sc, e->op);
        Sym *sym = d ? NULL : module_lookup(cur_module, e->op);
        if (sym) d = sym->def;
        if (!d) semantic_error(e, "Unknown operator: `%s'.", S(e->op));
        check_arity(e, d, e->n);
        e->def = d;
        if (d->kind == D_PARAM || d->kind == D_CONST) e->kind = K_PARAMCALL;
        else if (sym && sym->npath) {
            e->kind = K_INSTCALL;
            e->defs = sym->path;
            e->ndefs = sym->npath;
            e->segs = NEWN(Seg, sym->npath);
            e->nseg = sym->npath;
        }
        else e->kind = K_CALL;
        return;
    }
    case X_FORALL: case X_EXISTS: case X_CHOOSE: case X_FILTER: case X_MAP: case X_FCN:
    case X_TFORALL: case X_TEXISTS: {
        Scope *inner = sc;
        resolve_bounds(e, &inner);
        resolve_args(e->args, e->n, inner);
        check_temporal(e);
        return;
    }
    case X_LET: {
        Scope *inner = sc;
        for (int i = 0; i < e->ndefs; i++)
            if (e->defs[i]->recursive) inner = bind(inner, e->defs[i]->name, e->defs[i]);
        for (int i = 0; i < e->ndefs; i++) {
            Def *d = e->defs[i];
            d->local = 2;   /* a LET definition: bound in the environment */
            resolve_def(d, inner);
            if (!d->recursive) inner = bind(inner, d->name, d);
        }
        resolve_args(e->args, e->n, inner);
        return;
    }
    case X_EXCEPT:
        resolve_args(e->args, e->n, sc);
        for (int i = 0; i < e->n - 1; i++) resolve_args(e->paths[i].keys, e->paths[i].n, sc);
        return;
    default:
        resolve_args(e->args, e->n, sc);
        return;
    }
}

static void resolve_def(Def *d, Scope *sc) {
    d->mod = cur_module;
    Scope *inner = sc;
    for (int i = 0; i < d->nparams; i++) {
        d->params[i]->var_index = -1;
        inner = bind(inner, d->params[i]->name, d->params[i]);
    }
    if (d->kind == D_FCN) {
        Expr tmp;
        memset(&tmp, 0, sizeof tmp);
        tmp.bounds = d->bounds;
        tmp.nb = d->nb;
        resolve_bounds(&tmp, &inner);
        inner = bind(inner, d->name, d);
    }
    if (d->kind == D_INST) {
        int name = (int)(intptr_t)d->target;
        Module *t = load_module(name, cur_module);
        Module *saved = cur_module;
        resolve_module(t);
        cur_module = saved;
        d->target = t;
        Def **decls = NEWN(Def *, t->ndecls);
        Expr **exprs = NEWN(Expr *, t->ndecls);
        for (int i = 0; i < t->ndecls; i++) {
            Def *decl = t->decls[i];
            decls[i] = decl;
            for (int j = 0; j < d->nsubst; j++)
                if (d->subst_decl[j]->name == decl->name) exprs[i] = d->subst_expr[j];
            if (!exprs[i]) {
                Expr *ref = NEW(Expr);
                if (d->where) *ref = *d->where;
                ref->kind = decl->arity && !scope_lookup(inner, decl->name) ? X_OPREF : X_NAME;   /* Op(_) <- Op */
                ref->nseg = 1;
                ref->segs = NEW(Seg);
                ref->segs->name = decl->name;
                ref->op = decl->name;
                if (!scope_lookup(inner, decl->name) && !module_lookup(cur_module, decl->name))
                    semantic_error(d->where, "The constant or variable %s of module %s is not defined in the instantiating module.",
                                   S(decl->name), S(t->name));
                exprs[i] = ref;
            }
            resolve_expr(exprs[i], inner);
        }
        d->nsubst = t->ndecls;
        d->subst_decl = decls;
        d->subst_expr = exprs;
        return;
    }
    int open = d->kind == D_FCN && nopen_defs < 64;
    if (open) open_defs[nopen_defs++] = d;
    if (d->body) resolve_expr(d->body, inner);
    nopen_defs -= open;
}

static void resolve_module(Module *m) {
    if (m->resolved == 1) {
        printf("Circular dependency among .tla files: module %s depends on itself.\n", S(m->name));
        printf("Error: Parsing or semantic analysis failed.\n");
        exit(TPP_ERROR_PARSE);
    }
    if (m->resolved) return;
    m->resolved = 1;
    Module *saved = cur_module;
    for (int i = 0; i < m->nextends; i++) {
        Module *x = load_module(m->extends[i], m);
        resolve_module(x);
    }
    cur_module = m;
    for (int i = 0; i < m->nextends; i++) {
        Module *x = load_module(m->extends[i], m);
        for (int k = 0; k < x->nsyms; k++)
            if (!x->syms[k].local && !inherited_conflict(m, x->syms[k].def, NULL))
                add_sym(m, x->syms[k].name, x->syms[k].def, x->syms[k].path, x->syms[k].npath, 0, NULL);
        for (int k = 0; k < x->ndecls; k++) add_decl(m, x->decls[k]);
    }
    /* A name is defined before it is used; RECURSIVE declares a definition to come. */
    for (int i = 0; i < m->nunits; i++) {
        Unit *u = &m->units[i];
        cur_module = m;
        if (u->kind == 0) {
            Def *d = u->def;
            if (d->kind == D_CONST || d->kind == D_VAR) {
                d->mod = m;
                check_outer(m, d);
                add_sym(m, d->name, d, NULL, 0, 0, d->where);
                add_decl(m, d);
            } else if (d->kind == D_OP || d->kind == D_FCN || d->kind == D_INST) {
                resolve_def(d, NULL);
                check_outer(m, d);
                add_sym(m, d->name, d, NULL, 0, u->local, d->where);
            } else {
                resolve_def(d, NULL);
            }
        } else if (u->kind == 4 && u->def->body) {
            /* A theorem's or assumption's name is an operator without parameters. */
            Def *d = NEW(Def);
            *d = *u->def;
            d->kind = D_OP;
            d->level = d->var_index = -1;
            resolve_def(d, NULL);
            add_sym(m, d->name, d, NULL, 0, 0, NULL);
        } else if (u->kind == 4 && u->def->kind == D_RECURSIVE) {
            for (int k = i + 1; k < m->nunits; k++)
                if (m->units[k].kind == 0 && m->units[k].def->name == u->def->name &&
                    (m->units[k].def->kind == D_OP || m->units[k].def->kind == D_FCN)) {
                    add_sym(m, u->def->name, m->units[k].def, NULL, 0, m->units[k].local, m->units[k].def->where);
                    break;
                }
        } else if (u->kind == 1) {
            Def *d = u->def;
            resolve_def(d, NULL);
            cur_module = m;
            Module *t = d->target;
            for (int k = 0; k < t->nsyms; k++) {
                Sym *s = &t->syms[k];
                if (s->local || s->def->kind == D_CONST || s->def->kind == D_VAR) continue;
                if (inherited_conflict(m, s->def, d)) continue;
                Def **path = NEWN(Def *, s->npath + 1);
                path[0] = d;
                if (s->npath) memcpy(path + 1, s->path, sizeof(Def *) * (size_t)s->npath);
                add_sym(m, s->name, s->def, path, s->npath + 1, u->local, NULL);
            }
        } else if (u->kind == 2 && !u->def) {   /* a named one is resolved with its name */
            resolve_expr(u->expr, NULL);
        } else if (u->kind == 3) {
            u->module->nouter_syms = m->nsyms;
            resolve_module(u->module);
        }
    }
    for (int i = 0; i < m->nunits; i++) {
        Unit *u = &m->units[i];
        if (u->kind == 2) check_levels(u->expr, 3);
        else if (u->kind == 4 && u->def->body) check_levels(u->def->body, 3);
        else if (u->kind == 1 || (u->kind == 0 && u->def->kind != D_CONST && u->def->kind != D_VAR)) check_def_levels(u->def);
    }
    if (!m->outer) print_warnings(m);
    m->resolved = 2;
    cur_module = saved;
}

/* Level checking (§5.8). Each parameter of a definition, and each
   declaration of a module, may be used only where its argument or
   substitute may have some level; an operator applied to arguments of
   higher levels than its parameters allow is an error. */

static int level_max(Def *d) { return d->level_max ? d->level_max - 1 : 3; }

/* Argument i of op in e, where its level may be at most cap; e itself may be at most max. */
static void check_argument(Expr *e, const char *op, int i, Expr *a, int cap, int max) {
    if (!a) return;
    if (expr_level(a) > cap)
        semantic_error(e, "Level error in applying operator %s:\nThe level of argument %d exceeds the maximum level allowed by the operator.", op, i + 1);
    check_levels(a, cap < max ? cap : max);
}

/* The operands of e, the first of level at most first, the others at most rest. */
static void check_operands(Expr *e, int first, int rest, int max) {
    for (int i = 0; i < e->n; i++) check_argument(e, S(e->op), i, e->args[i], i ? rest : first, max);
}

/* SANY's name for a built-in operator whose arguments may be at most
   actions; the body of a function constructor or a quantifier may be
   temporal, as their bounds may not. */
static const char *sany_op(Expr *e) {
    switch (e->kind) {
    case K_EQ: return "=";
    case K_NEQ: return "/=";
    case K_IN: return "\\in";
    case K_NOTIN: return "\\notin";
    case K_CUP: return "\\union";
    case K_CAP: return "\\intersect";
    case K_SETMINUS: return "\\";
    case K_SUBSETEQ: return "\\subseteq";
    case K_SUBSET: return "SUBSET";
    case K_UNION: return "UNION";
    case K_DOMAIN: return "DOMAIN";
    case K_TIMES: return "$CartesianProd";
    case X_SETENUM: return "$SetEnumerate";
    case X_TUPLE: return "$Tuple";
    case X_RCDSET: return "$SetOfRcds";
    case X_FCNSET: return "$SetOfFcns";
    case X_APPLY: return "$FcnApply";
    case X_DOT: return "$RcdSelect";
    case X_EXCEPT: return "$Except";
    case X_CHOOSE: return e->bounds[0].set ? "$BoundedChoose" : "$UnboundedChoose";
    case X_FILTER: return "$SubsetOf";
    case X_MAP: return "$SetOfAll";
    case X_FCN: return "$FcnConstructor";
    case X_FORALL: return "$BoundedForall";
    case X_EXISTS: return "$BoundedExists";
    default: return NULL;
    }
}

/* SANY's name for a connective that may not join a temporal formula and an action. */
static const char *sany_connective(int kind) {
    switch (kind) {
    case K_AND: return "\\land";
    case K_OR: return "\\lor";
    case K_IMPLIES: return "=>";
    case K_EQUIV: return "\\equiv";
    default: return NULL;
    }
}

/* Checks e, where its level may be at most max, and records that limit for
   the parameters and declarations it uses. */
static void check_levels(Expr *e, int max) {
    if (!e) return;
    Def *d = e->def;
    switch (e->kind) {
    case K_BOUND: case K_DECL:
        if (max < level_max(d)) d->level_max = max + 1;
        return;
    case K_PRIME: case K_UNCHANGED: check_operands(e, 1, 1, max); return;
    case K_ENABLED: check_operands(e, 2, 2, 2); return;   /* a state predicate of any action */
    case K_CDOT: check_operands(e, 2, 2, max); return;
    case X_SQACT: case X_ANGLEACT: check_operands(e, 2, 1, max); return;   /* [A]_v */
    case X_WF: case X_SF: check_operands(e, 1, 2, max); return;            /* WF_v(A) */
    case K_CALL: case K_INSTCALL:
        check_def_levels(d);
        for (int i = 0; i < e->n; i++)
            check_argument(e, S(d->name), i, e->args[i], i < d->nparams ? level_max(d->params[i]) : 3, max);
        for (int k = 0; e->kind == K_INSTCALL && k < e->ndefs; k++) {
            Def *in = e->defs[k];
            check_def_levels(in);
            for (int i = 0; i < e->segs[k].n && i < in->nparams; i++)
                check_argument(e, S(in->name), i, e->segs[k].args[i], level_max(in->params[i]), max);
        }
        return;
    case X_LET:
        for (int i = 0; i < e->ndefs; i++) check_def_levels(e->defs[i]);
        check_levels(e->args[0], max);
        return;
    case X_LAMBDA: check_def_levels(d); return;
    default: {
        const char *op = sany_op(e), *conn = sany_connective(e->kind);
        int inner = op && max > 2 ? 2 : max;
        for (int i = 0; i < e->nb; i++) {
            Bound *b = &e->bounds[i];
            if (op && b->set && expr_level(b->set) > 2) {
                Expr at = {.mod = e->mod, .l1 = b->line, .c1 = b->col, .l2 = b->line, .c2 = b->col + (int)strlen(S(b->names[0])) - 1};
                Buf loc = {0};
                loc_text(&loc, &at);
                semantic_error(e, "Level error in applying operator %s:\nThe level of the range for the bounded variable %s \nexceeds the maximum level allowed by the operator.", op, loc.s);
            }
            check_levels(b->set, inner);
        }
        for (int i = 0; e->kind == X_EXCEPT && i < e->n - 1; i++) {
            Path *p = &e->paths[i];
            Expr at = *e->args[i + 1];   /* the update, from its ! to its value's end */
            at.l1 = p->line;
            at.c1 = p->col;
            for (int k = 0; k < p->n; k++) check_argument(&at, "$Seq", k, p->keys[k], 2, max);
        }
        if (conn) {
            int temporal = 0, action = 0;
            for (int i = 0; i < e->n; i++) { int l = expr_level(e->args[i]); temporal |= l == 3; action |= l == 2; }
            if (temporal && action) semantic_error(e, "%s has both temporal formula and action as arguments.", conn);
        }
        int free = e->kind == X_FCN || e->kind == X_FORALL || e->kind == X_EXISTS;
        for (int i = 0; i < e->n; i++)
            if (op && !free) check_argument(e, op, i, e->args[i], 2, max);
            else check_levels(e->args[i], max);
        return;
    }
    }
}

/* A definition, once: its body, or an instance's substitutions, each of a
   level its declaration allows. */
static void check_def_levels(Def *d) {
    if (d->level_checked) return;
    d->level_checked = 1;
    if (d->kind == D_INST) {
        for (int i = 0; i < d->nsubst; i++) {
            Def *decl = d->subst_decl[i];
            Expr *x = d->subst_expr[i];
            if (expr_level(x) > level_max(decl))
                semantic_error(d->where, "Level error in instantiating module '%s':\nThe level of the expression or operator substituted for '%s' \nmust be at most %d.",
                               S(d->target->name), S(decl->name), level_max(decl));
            check_levels(x, level_max(decl));
        }
        return;
    }
    for (int i = 0; i < d->nb; i++) check_levels(d->bounds[i].set, 3);
    check_levels(d->body, 3);
}

/* The level of an expression (0 constant, 1 state, 2 action, 3 temporal). */

/* A parameter whose uses are given level 4, to find whether the level of
   an argument for it reaches the level of a call. */
static Def *level_probe;

/* Levels as TLC bounds them to decide what to evaluate once: every
   argument of a call counts, used or not. */
static int every_argument;

static int def_level(Def *d) {
    if (every_argument ? d->level_bound > 0 : d->level >= 0 && (d->local != 2 || d->level_probe == level_probe))
        return every_argument ? d->level_bound - 1 : d->level;
    if (d->level_busy) return 0;
    d->level_busy = 1;
    int l = 0;
    if (d->kind == D_VAR) l = 1;
    else if (d->kind == D_OP || d->kind == D_FCN) {
        if (d->body) l = expr_level(d->body);
        for (int i = 0; i < d->nb; i++) if (d->bounds[i].set) { int b = expr_level(d->bounds[i].set); if (b > l) l = b; }
    } else if (d->kind == D_INST) {
        for (int i = 0; i < d->nsubst; i++)
            if (d->subst_expr[i]->kind != X_OPREF) { int b = expr_level(d->subst_expr[i]); if (b > l) l = b; }
    }
    d->level_busy = 0;
    if (every_argument) d->level_bound = l + 1;
    else if (l < 4) {   /* else the probe reached it */
        d->level = l;
        d->level_probe = level_probe;   /* a LET definition may use the probe */
    }
    return l;
}

static int max_level(Expr **args, int n);

/* The level of a call of d: an argument counts when its parameter is used
   outside ENABLED, as SANY finds the level parameters of a definition. */
static int args_level(Def *d, Expr **args, int n) {
    if (every_argument) return max_level(args, n);
    if (!d->level_mask && d->body) {
        d->level_mask = ~0u;   /* a recursive call: every argument counts */
        unsigned m = 0;
        Def *outer = level_probe;
        for (int i = 0; i < d->nparams && i < 31; i++) {
            level_probe = d->params[i];
            if (expr_level(d->body) == 4) m |= 1u << i;
        }
        level_probe = outer;
        d->level_mask = 1 | m << 1;
    }
    int l = 0;
    for (int i = 0; i < n; i++)
        if (!d->body || i >= 31 || d->level_mask >> (i + 1) & 1) { int a = expr_level(args[i]); if (a > l) l = a; }
    return l;
}

static int max_level(Expr **args, int n) {
    int l = 0;
    for (int i = 0; i < n; i++) if (args[i]) { int a = expr_level(args[i]); if (a > l) l = a; }
    return l;
}

/* Whether e's level depends on a constant or variable: SANY's
   levelParams are not empty. A prime or a temporal operator fixes the
   level of what it applies to; an operator's argument counts if its
   parameter does. TLC says more about an invariant that is no state
   predicate when they are not empty. */
static Def *param_probe;
static int level_params(Expr *e) {
    if ((e->kind == K_BOUND || e->kind == K_PARAMCALL) && e->def == param_probe) return 1;
    switch (e->kind) {
    case K_DECL: return 1;
    case K_PRIME: case K_UNCHANGED: case X_SQACT: case X_ANGLEACT: case K_ENABLED:
    case K_ALWAYS: case K_EVENTUALLY: case K_LEADSTO: case K_WHILE: case X_WF: case X_SF:
    case X_TFORALL: case X_TEXISTS: case X_LAMBDA: case X_OPREF:
        return 0;
    case K_CALL: case K_INSTCALL: {
        Def *d = e->def;
        if (!d->body) break;
        if (d->level_busy) return 0;
        d->level_busy = 1;
        int found = level_params(d->body);
        Def *outer = param_probe;
        for (int i = 0; i < e->n && i < d->nparams && !found; i++) {
            param_probe = d->params[i];
            found = level_params(d->body) && (param_probe = outer, level_params(e->args[i]));
        }
        param_probe = outer;
        d->level_busy = 0;
        return found;
    }
    }
    for (int i = 0; i < e->n; i++) if (level_params(e->args[i])) return 1;
    for (int i = 0; i < e->nb; i++) if (e->bounds[i].set && level_params(e->bounds[i].set)) return 1;
    if (e->kind == X_EXCEPT)
        for (int i = 0; i < e->n - 1; i++)
            for (int k = 0; k < e->paths[i].n; k++) if (level_params(e->paths[i].keys[k])) return 1;
    return 0;
}

static int expr_level(Expr *e) {
    if ((e->kind == K_BOUND || e->kind == K_PARAMCALL) && level_probe && e->def == level_probe) return 4;
    int l = e->kind == K_CALL || e->kind == K_INSTCALL ? args_level(e->def, e->args, e->n) : max_level(e->args, e->n);
    for (int i = 0; i < e->nb; i++) if (e->bounds[i].set) { int b = expr_level(e->bounds[i].set); if (b > l) l = b; }
    switch (e->kind) {
    case K_DECL: return e->def->kind == D_VAR ? 1 : 0;
    case K_PRIME: case K_UNCHANGED: case X_SQACT: case X_ANGLEACT: case K_CDOT: return l < 2 ? 2 : l;
    case K_ENABLED: return 1;
    case K_ALWAYS: case K_EVENTUALLY: case K_LEADSTO: case X_WF: case X_SF: case K_WHILE:
    case X_TFORALL: case X_TEXISTS:
        return 3;
    case K_CALL: case K_PARAMCALL: {
        int d = def_level(e->def);
        return d > l ? d : l;
    }
    case K_INSTCALL: {
        int d = def_level(e->def);
        for (int i = 0; i < e->ndefs; i++) { int p = def_level(e->defs[i]); if (p > d) d = p; }
        for (int i = 0; i < e->nseg; i++) { int a = max_level(e->segs[i].args, e->segs[i].n); if (a > d) d = a; }
        return d > l ? d : l;
    }
    case X_LET:
        for (int i = 0; i < e->ndefs; i++) { int d = def_level(e->defs[i]); if (d > l) l = d; }
        return l;
    case X_EXCEPT:
        for (int i = 0; i < e->n - 1; i++) { int k = max_level(e->paths[i].keys, e->paths[i].n); if (k > l) l = k; }
        return l;
    case X_LAMBDA: return expr_level(e->def->body);
    case X_OPREF: return def_level(e->def);
    default: return l;
    }
}

/* ---------------------------------------------------------------- evaluator */

static int nvars;
static Def **vars;              /* the state variables, in TLC's order */
static Value **cur_state;       /* the state being read */
static Value **next_state;      /* the next state an action assigns; NULL entries unassigned */
static int primed;              /* reading the next state */
static int gen_init;            /* assigning unprimed variables: an initial predicate */
static long state_reads;        /* incremented by each read of a variable */
static long moving_reads;       /* by each read of one that may change: primed, or in Init */
static Value *at_value;         /* @ in an EXCEPT */
static int keep_lazy;           /* TLC's KeepLazy: evaluating a function to apply it (§6.6) */
static int64_t cur_level;       /* TLCGet("level") */
static int eval_depth;

/* A definition's body is entered: only calls nest without bound. Past
   the bound, TLC's Java stack has overflowed. */
static void deeper(void) {
    if (++eval_depth <= 5000) return;
    eval_depth = 0;
    error_reset();
    buf_puts(&error_message, "This was a Java StackOverflowError. It was probably the result\n"
             "of an incorrect recursive function definition that caused TLC to enter\n"
             "an infinite loop when trying to compute the function or its application\n"
             "to an element in its putative domain.");
    stackless = 2;
    raise_error();
}

static Value *eval_body(Expr *body, Ctx c) {
    deeper();
    Value *v = eval(body, c);
    eval_depth--;
    return v;
}

static Value *boolean_set;

static Env *mk_binding(Def *d, Value *v, Env *next) {
    Env *b = NEW(Env);
    b->def = d; b->v = v; b->next = next;
    b->perm = arena == &perm_arena;
    return b;
}

/* The variables of bounds, a tuple pattern counting one. */
static int bounds_total(Bound *bs, int nb) {
    int total = 0;
    for (int i = 0; i < nb; i++) total += bs[i].tuple ? 1 : bs[i].n;
    return total;
}

/* The order in which TLC enumerates the domain of a function of several
   bounds, sorted in keys: the first bound varies fastest, so the tuples
   compare from their last part. */
static Value **order_keys;
static int params_cmp(const void *x, const void *y) {
    Value *a = order_keys[*(const int *)x], *b = order_keys[*(const int *)y];
    for (int i = a->n - 1; i >= 0; i--) {
        int c = vcmp(a->u.fcn.vals[i], b->u.fcn.vals[i]);
        if (c) return c;
    }
    return 0;
}

static int *params_order(Value **keys, int n) {
    int *at = NEWN(int, n);
    for (int i = 0; i < n; i++) at[i] = i;
    order_keys = keys;
    qsort(at, (size_t)n, sizeof *at, params_cmp);
    return at;
}


/* The domain of [x \in S, <<y, z>> \in T |-> e]: the bound set, or the
   product of one set per variable (a tuple pattern is one). */
static Value *bounds_domain(Bound *bs, int nb, Ctx c) {
    int total = 0, k = 0;
    for (int i = 0; i < nb; i++) {
        if (!bs[i].set) fail("TLC cannot handle the unbounded quantifier or CHOOSE over %s.", S(bs[i].names[0]));
        total += bs[i].tuple ? 1 : bs[i].n;
    }
    Value *dom = mk_lazy(L_TIMES);
    dom->n = total;
    dom->u.l.names = NEWN(Value *, 2 * total);
    for (int i = 0; i < nb; i++) {
        Value *set = eval(bs[i].set, c);
        for (int j = 0; j < (bs[i].tuple ? 1 : bs[i].n); j++) dom->u.l.names[total + k++] = set;
    }
    return total == 1 ? dom->u.l.names[1] : dom;
}

/* Whether TLC computes a function over a domain bounds_domain made at
   once: if each bound's set is reducible (§6.6). */
static int bounds_reducible(Value *dom, int total) {
    if (total == 1) return reducible(dom);
    for (int k = 0; k < total; k++) if (!reducible(dom->u.l.names[total + k])) return 0;
    return 1;
}

/* Binds the variables of the bounds to the parts of a domain element. */
static Env *bind_key(Bound *bs, int nb, Value *key, Env *env, Expr *where) {
    int total = bounds_total(bs, nb), k = 0;
    for (int i = 0; i < nb; i++) {
        Bound *b = &bs[i];
        for (int j = 0; j < (b->tuple ? 1 : b->n); j++, k++) {
            Value *part = total == 1 ? key : fcn_apply(key, mk_int(k + 1), where);
            if (!b->tuple) { env = mk_binding(b->defs[j], part, env); continue; }
            for (int m = 0; m < b->n; m++) env = mk_binding(b->defs[m], fcn_apply(part, mk_int(m + 1), where), env);
        }
    }
    return env;
}

/* In a simulation a state may hold a lazy value, which outlives the states
   it was made in; it is evaluated in those states, as TLC does. */
static int simulating;

static Value **snapshot(Value **st) {
    return st ? memcpy(NEWN(Value *, nvars), st, sizeof(Value *) * (size_t)nvars) : NULL;
}

/* Copies: the states being generated change as generation goes on. */
static Value ***made_in(void) {
    if (!simulating) return NULL;
    Value ***s = NEWN(Value **, 2);
    s[0] = snapshot(cur_state);
    s[1] = snapshot(next_state);
    return s;
}

/* A function kept as its definition: [x \in S |-> e] (expr) or a
   function definition (def), over an infinite domain or made lazy (§6.6). */
static Value *mk_lambda(Value *dom, Expr *e, Def *d, Ctx c) {
    Value *f = NEW(Value);
    f->kind = V_LAMBDA;
    f->u.l.a = dom;
    f->u.l.expr = e;
    f->u.l.b = at_value;
    f->u.l.binder = d;
    f->u.l.env = c.env;
    f->u.l.inst = c.inst;
    f->keep = keep_lazy || (d && d->self);
    f->perm = arena == &perm_arena;
    f->u.l.made_in = made_in();
    return f;
}

static Expr *fcndef_bind(Def *d, Value *a, Ctx *c, Expr *where);
static int bind_arg(Bound *bs, int nb, Value *dom, Value *a, Env **env, int *argno);
static _Noreturn void lambda_outside(Value *f, Value *a, int why, int argno, Expr *where);

static Value *lambda_body(Value *f, Value *a);

static Value *lambda_apply(Value *f, Value *a) {
    for (int i = f->n - 1; i >= 0; i--)
        if (veq(f->u.l.names[2 * i], a)) return f->u.l.names[2 * i + 1];
    if (!f->u.l.made_in) return lambda_body(f, a);
    Value **cur = cur_state, **next = next_state;
    cur_state = f->u.l.made_in[0];
    next_state = f->u.l.made_in[1];
    Value *v = lambda_body(f, a);
    cur_state = cur;
    next_state = next;
    return v;
}

/* The body of lazy function f, its bound names given a in *c; where is
   the function, when computing states. */
static Expr *lambda_bind(Value *f, Value *a, Ctx *c, Expr *where) {
    *c = (Ctx){f->u.l.env, f->u.l.inst};
    if (f->u.l.binder) return fcndef_bind(f->u.l.binder, a, c, where);
    Expr *e = f->u.l.expr;
    int argno, why = bind_arg(e->bounds, e->nb, f->u.l.a, a, &c->env, &argno);
    if (why) lambda_outside(f, a, why, argno, where);
    return e->args[0];
}

static Value *eval_body(Expr *body, Ctx c);

static Value *lambda_body(Value *f, Value *a) {
    Ctx c;
    Expr *body = lambda_bind(f, a, &c, NULL);
    Value *saved = at_value;
    at_value = f->u.l.b;
    Value *v = eval_body(body, c);
    at_value = saved;
    return v;
}

/* The function a lazy one is, which it becomes in place where the arenas
   allow; one over an infinite domain stays as it is. */
static __attribute__((cold)) Value *realize_lambda(Value *f) {
    if (set_is_infinite(f->u.l.a)) return f;
    Arena *saved = keeping(f->perm);
    Value *dom = enumerate(f->u.l.a);
    Value *g = mk_fcn(dom->n);
    memcpy(g->u.fcn.keys, dom->u.set.elems, sizeof(Value *) * (size_t)dom->n);
    Def *d = f->u.l.binder;
    int params = d ? bounds_total(d->bounds, d->nb) > 1 : bounds_total(f->u.l.expr->bounds, f->u.l.expr->nb) > 1;
    int *at = params ? params_order(g->u.fcn.keys, dom->n) : NULL;
    int keep = keep_lazy;
    keep_lazy = f->keep;
    for (int j = 0; j < dom->n; j++) {
        int i = at ? at[j] : j;
        g->u.fcn.vals[i] = lambda_apply(f, dom->u.set.elems[i]);
    }
    keep_lazy = keep;
    g->made = MADE_LAMBDA;
    if (may_keep(f)) { *f = *g; g = f; }
    arena = saved;
    return g;
}

static Env *mk_thunk(Def *d, Expr *e, Ctx c, Env *next) {
    Env *b = NEW(Env);
    b->def = d; b->e = e; b->ctx_env = c.env; b->ctx_inst = c.inst; b->next = next;
    b->perm = arena == &perm_arena;
    return b;
}

static Env *env_find(Env *env, Def *d) {
    for (; env; env = env->next) if (env->def == d) return env;
    return NULL;
}

/* An argument for parameter d: a thunk, or the value of a bound name that
   already has one that no state changes, as forcing the thunk would give. */
static Env *mk_arg(Def *d, Expr *e, Ctx c, Env *next) {
    if (e->kind == K_BOUND) {
        Env *b = env_find(c.env, e->def);
        if (b && b->v && !b->stateful) return mk_binding(d, b->v, next);
    }
    return mk_thunk(d, e, c, next);
}

/* The value of a binding, evaluated once where that is safe: a value read
   from no state at all, or from the current state by a binding that lives
   no longer than one state's expansion, as the scratch arena does. */
static Value *force(Env *b) {
    if (b->v && !(primed && b->stateful)) return b->v;
    long reads = state_reads, moving = moving_reads;
    Ctx c = {b->ctx_env, b->ctx_inst};
    Arena *saved = keeping(b->perm);
    Value *v = eval(b->e, c);
    if (b->perm == (arena == &perm_arena)) {
        held_by(b);
        if (reads == state_reads) { b->v = v; b->stateful = 0; }
        else if (!b->perm && moving == moving_reads && !primed) { b->v = v; b->stateful = 1; }
    }
    arena = saved;
    return v;
}

static Value *mk_oper(Def *d, Ctx c) {
    Value *v = NEW(Value);
    v->kind = V_OPER;
    v->u.op.def = d;
    v->u.op.env = c.env;
    v->u.op.inst = c.inst;
    return v;
}

static int64_t check_int(int64_t v, int64_t a, int64_t b, const char *op) {
    if (v > 2147483647LL || v < -2147483648LL)
        fail_module("Overflow when computing %lld%s%lld", (long long)a, op, (long long)b);
    return v;
}

static int64_t int_arg(Value *v, int which, const char *op) {
    if (v->kind != V_INT)
        fail_module("The %s argument of %s should be an integer, but instead it is:\n%s", which == 1 ? "first" : "second", op, pretty(v));
    return v->u.i;
}

/* The Java class TLC keeps a value in, which its casting errors name. */
static const char *java_class(Value *v) {
    switch (v->kind) {
    case V_BOOL: return "BoolValue";
    case V_INT: return "IntValue";
    case V_STR: return "StringValue";
    case V_MODEL: return "ModelValue";
    case V_TUPLE: return "TupleValue";
    case V_FCN: return v->made == MADE_RECORD ? "RecordValue" : "FcnRcdValue";
    case V_SET: return "SetEnumValue";
    case V_INTERVAL: return "IntervalValue";
    case V_LAMBDA: return "FcnLambdaValue";
    case V_OPER: return "OpLambdaValue";
    default: break;
    }
    switch (v->lazy) {
    case L_SUBSET: return "SubsetValue";
    case L_FCNSET: return "SetOfFcnsValue";
    case L_RCDSET: return "SetOfRcdsValue";
    case L_FILTER: return "SetPredValue";
    case L_TIMES: return "SetOfTuplesValue";
    case L_UNION: return "UnionValue";
    case L_CUP: return "SetCupValue";
    case L_CAP: return "SetCapValue";
    case L_DIFF: return "SetDiffValue";
    default: return "UserValue";
    }
}

/* An argument of an arithmetic operator, which TLC casts to an integer. */
static int64_t int_cast(Value *v) {
    if (v->kind != V_INT) fail("Cannot cast tlc2.value.impl.%s to tlc2.value.impl.IntValue", java_class(v));
    return v->u.i;
}

static Value *eval_operator_arg(Expr *a, Ctx c);
static Value *apply_oper(Value *op, Value **args, int n, Expr *where);
static Value *call_def(Def *d, Expr **args, int n, Ctx c, Expr *call);
static int enabled(Expr *a, Ctx c);

/* The value of a constant or variable, through the substitutions of the
   instances the context is in (§4.3). */
/* The expression an instance substitutes for d, if any; c becomes the
   context it is in. */
static Expr *substitute(Def *d, Ctx *c) {
    for (Inst *in = c->inst; in; in = in->parent)
        for (int i = 0; i < in->def->nsubst; i++)
            if (in->def->subst_decl[i] == d) { *c = (Ctx){in->env, in->parent}; return in->def->subst_expr[i]; }
    return NULL;
}

static Value *decl_value(Def *d, Ctx c, Expr *where) {
    Ctx outer = c;
    Expr *x = substitute(d, &outer);
    if (x) {
        if (x->kind == X_OPREF) return x->def->kind == D_CONST ? decl_value(x->def, outer, where) : mk_oper(x->def, outer);
        return d->arity ? eval_operator_arg(x, outer) : eval(x, outer);
    }
    if (d->kind == D_VAR) {
        if (d->var_index < 0) fail_at(where, "The variable %s is not a variable of the specification.", S(d->name));
        state_reads++;
        if (primed || gen_init) moving_reads++;
        if (primed) {
            if (!next_state || !next_state[d->var_index])
                fail_runtime(where, TPP_ERROR_CONFIG, "In evaluation, the identifier %s is either undefined or not an operator.", S(d->name));
            return next_state[d->var_index];
        }
        if (!cur_state || !cur_state[d->var_index])
            fail_runtime(where, TPP_ERROR_CONFIG, "In evaluation, the identifier %s is either undefined or not an operator.", S(d->name));
        return cur_state[d->var_index];
    }
    if (d->replacement) return call_def(d->replacement, NULL, 0, (Ctx){NULL, NULL}, where);
    if (!d->value) fail_at(where, "The constant parameter %s is not assigned a value by the configuration file.", S(d->name));
    return d->value;
}

/* The definition a constant operator stands for, by the substitution of
   an instance the context is in or by the configuration, with the context
   it is in; or NULL. */
static Def *const_operator(Def *d, Ctx c, Ctx *at) {
    for (Inst *in = c.inst; in; in = in->parent) {
        Def *id = in->def;
        for (int i = 0; i < id->nsubst; i++)
            if (id->subst_decl[i] == d) {
                Expr *x = id->subst_expr[i];
                *at = (Ctx){in->env, in->parent};
                if (x->kind != X_OPREF) return NULL;
                return x->def->kind == D_CONST ? const_operator(x->def, *at, at) : x->def;
            }
    }
    return d->replacement;
}

/* Bindings of `x \in S, <<a, b>> \in T`: calls fn for each, in TLC's
   order, until it returns nonzero. */
typedef int (*BindFn)(Ctx c, void *data);

/* TLC's error for a quantifier over s: Nat, Int and Seq(S) explain
   themselves after the bound's location. */
static _Noreturn void non_enumerable_bound(Value *s, Expr *bound) {
    const char *infinite = s->kind != V_LAZY ? NULL : s->lazy == L_NAT ? "Nat" : s->lazy == L_INT ? "Int" : NULL;
    if (!infinite && !(s->kind == V_LAZY && s->lazy == L_SEQ))
        fail_at(bound, "TLC encountered a non-enumerable quantifier bound\n%s.", pretty(s));
    Buf b = {0};
    const char *v = pretty(s);
    buf_printf(&b, "TLC encountered the non-enumerable quantifier bound\n%s", v);
    loc_line(&b, bound);
    if (infinite)
        buf_printf(&b, "\nThe set %s contains infinitely many elements. As a result, TLC cannot evaluate expressions that\n"
                   "universally (\\A) or existentially (\\E) quantify over %s, because this would require checking an\n"
                   "infinite number of cases. Note that TLC handles set membership like T \\subseteq %s for any finite set T.",
                   infinite, v, infinite);
    else
        buf_printf(&b, "\nIn TLA+, Seq(S) represents the set of all finite sequences whose elements come from the set S. Even when S\n"
                   "is a finite set, the number of possible sequences in Seq(S) is unbounded because sequences can have any\n"
                   "finite length (e.g., length 0, 1, 2, and so on). As a result, TLC cannot evaluate expressions that\n"
                   "universally (\\A) or existentially (\\E) quantify over %s, because this would require checking an\n"
                   "infinite number of cases. Note that for a finite set of sequences s, TLC handles s \\subseteq Seq(S).\n"
                   "See https://explain.tlapl.us/seq-unenumerable for additional details.", v);
    fail("%s", b.s);
}

/* The set a bound ranges over, enumerated. */
static Value *bound_value(Bound *b, Ctx c) {
    if (!b->set) fail("TLC cannot handle the unbounded quantifier or CHOOSE over %s.", S(b->names[0]));
    Value *s = eval(b->set, c);
    if (!is_set(s) || user_set(s)) non_enumerable_bound(s, b->set);
    return s;
}

static Value *bound_set(Bound *b, Ctx c) { return elements(bound_value(b, c)); }

/* As TLC, the sets are enumerated first, and the first variable varies
   fastest. */
static int for_bindings(Bound *bs, int nb, Ctx c, BindFn fn, void *data) {
    if (nb == 1 && bs[0].n == 1 && !bs[0].tuple) {
        /* x \in S, the common case; an interval, as TLC, one by one. */
        Value *s = bound_value(&bs[0], c);
        if (s->kind == V_INTERVAL) {
            for (int64_t x = s->u.range.lo; x <= s->u.range.hi; x++)
                if (fn((Ctx){mk_binding(bs[0].defs[0], mk_int(x), c.env), c.inst}, data)) return 1;
            return 0;
        }
        s = elements(s);
        for (int i = 0; i < s->n; i++)
            if (fn((Ctx){mk_binding(bs[0].defs[0], s->u.set.elems[i], c.env), c.inst}, data)) return 1;
        return 0;
    }
    int total = bounds_total(bs, nb);
    if (total == 0) return fn(c, data);
    Bound **owner = NEWN(Bound *, total);
    Value **sets = NEWN(Value *, total);
    int *slot = NEWN(int, total), *index = NEWN(int, total);
    int k = 0;
    for (int i = 0; i < nb; i++) {
        Value *s = bound_set(&bs[i], c);
        for (int j = 0; j < (bs[i].tuple ? 1 : bs[i].n); j++) {
            owner[k] = &bs[i]; slot[k] = j; sets[k] = s; index[k] = 0;
            if (!s->n) return 0;
            k++;
        }
    }
    for (;;) {
        Env *env = c.env;
        for (k = 0; k < total; k++) {
            Bound *b = owner[k];
            Value *v = sets[k]->u.set.elems[index[k]];
            if (!b->tuple) { env = mk_binding(b->defs[slot[k]], v, env); continue; }
            if (!is_fcn(v) || !fcn_is_tuple(v) || v->n != b->n)
                fail("A tuple pattern <<...>> \\in S was matched with %s.", pretty(v));
            for (int j = 0; j < b->n; j++) env = mk_binding(b->defs[j], v->u.fcn.vals[j], env);
        }
        if (fn((Ctx){env, c.inst}, data)) return 1;
        for (k = 0; k < total && ++index[k] == sets[k]->n; k++) index[k] = 0;
        if (k == total) return 0;
    }
}

typedef struct { Expr *body; int value; Expr *where; } QuantData;

/* e in c, dropping what computing it made where that is allowed. */
static int eval_bool_dropping(Expr *e, Ctx c) {
    Region *r = region_open();
    int holds = eval_bool(e, c);
    region_close(r);
    return holds;
}

static int forall_fn(Ctx c, void *data) {
    QuantData *q = data;
    if (!eval_bool_dropping(q->body, c)) { q->value = 0; return 1; }
    return 0;
}

static int exists_fn(Ctx c, void *data) {
    QuantData *q = data;
    if (eval_bool_dropping(q->body, c)) { q->value = 1; return 1; }
    return 0;
}

typedef struct { Expr *body; List out; } MapData;

static int map_fn(Ctx c, void *data) {
    MapData *d = data;
    list_add(&d->out, eval(d->body, c));
    return 0;
}

/* Whether v satisfies the predicate of a lazy filter. */
static Value *filter_member_test(Value *s, Value *v) {
    Expr *e = s->u.l.expr;
    Bound *b = &e->bounds[0];
    Env *env = s->u.l.env;
    if (b->tuple) {
        if (!is_fcn(v) || v->n != b->n) return FALSEV;
        for (int j = 0; j < b->n; j++) env = mk_binding(b->defs[j], v->u.fcn.vals[j], env);
    } else {
        env = mk_binding(b->defs[0], v, env);
    }
    Ctx c = {env, s->u.l.inst};
    Value **cur = cur_state, **next = next_state;
    if (s->u.l.made_in) cur_state = s->u.l.made_in[0], next_state = s->u.l.made_in[1];
    int holds = eval_bool(e->args[0], c);
    cur_state = cur;
    next_state = next;
    return BOOLV(holds);
}

/* A function definition f[x \in S] == e: its body, x given a in *c. */
static Expr *fcndef_bind(Def *d, Value *a, Ctx *c, Expr *where) {
    Env *env = NULL;
    if (d->local == 2) {
        Env *b = env_find(c->env, d);
        env = b ? b->ctx_env : c->env;
    }
    Value *dom = bounds_domain(d->bounds, d->nb, (Ctx){env, c->inst});
    int argno, why = bind_arg(d->bounds, d->nb, dom, a, &env, &argno);
    if (why) lambda_outside(mk_lambda(dom, NULL, d, *c), a, why, argno, where);
    c->env = env;
    return d->body;
}

/* The value of function definition d at a. */
static Value *fcndef_apply(Def *d, Value *a, Ctx c) {
    Expr *body = fcndef_bind(d, a, &c, NULL);
    return eval_body(body, c);
}

/* [x \in S |-> e], or the function definition d, computed in full over
   dom; an error meanwhile is one TLC cannot fingerprint at `where`. */
static Value *fcn_in_full(Value *dom, Expr *e, Def *d, Ctx c, Expr *where) {
    fp_enter(where);
    dom = enumerate(dom);
    Value *f = mk_fcn(dom->n);
    memcpy(f->u.fcn.keys, dom->u.set.elems, sizeof(Value *) * (size_t)dom->n);
    int params = d ? bounds_total(d->bounds, d->nb) > 1 : bounds_total(e->bounds, e->nb) > 1;
    int *at = params ? params_order(f->u.fcn.keys, dom->n) : NULL;
    for (int j = 0; j < dom->n; j++) {
        int i = at ? at[j] : j;
        Value *k = dom->u.set.elems[i];
        f->u.fcn.vals[i] = d ? fcndef_apply(d, k, c)
                             : eval(e->args[0], (Ctx){bind_key(e->bounds, e->nb, k, c.env, e), c.inst});
    }
    fp_leave(where);
    if (!d) f->made = MADE_LAMBDA;
    return f;
}

static Value *fcndef_value(Def *d, Ctx c) {
    Value *dom = bounds_domain(d->bounds, d->nb, c);
    if (d->self || keep_lazy || !bounds_reducible(dom, bounds_total(d->bounds, d->nb))) return mk_lambda(dom, NULL, d, c);
    return fcn_in_full(dom, NULL, d, c, d->where);
}

static int nowarning;   /* TLC's warnings off (-nowarning) */

/* Prints one of TLC's warnings; as TLC, the first of a run says how to
   turn them off. */
static int warned;
static void warning(const char *text) {
    if (nowarning) return;
    printf("Warning: %s\n", text);
    if (!warned++) printf("(Use the -nowarning option to disable this warning.)\n");
    fflush(stdout);
}

/* TLC's warning that an EXCEPT names a key outside the domain, each
   message once: the first, with the -nowarning line, is another. */
static void warn_except(Expr *e) {
    static char **seen;
    static int nseen;
    if (nowarning) return;
    if (in_worker) worker_cannot();   /* the search prints in order */
    Buf b = {0};
    buf_puts(&b, warned ? "+" : "-");
    buf_puts(&b, "The EXCEPT was applied to non-existing fields of the value at\n");
    loc_text(&b, e);
    for (int i = 0; i < nseen; i++)
        if (!strcmp(seen[i], b.s)) { free(b.s); return; }
    seen = xrealloc(seen, sizeof(char *) * (size_t)(nseen + 1));
    seen[nseen++] = b.s;
    warning(b.s + 1);
}

static Value *except_update(Value *f, Path *p, int i, Expr *val, Ctx c, Expr *where) {
    Value *key = p->fields[i] >= 0 ? mk_str(p->fields[i]) : eval(p->keys[i], c);
    if (!is_fcn(f) && f->kind != V_LAMBDA) fail_at(where, "Attempted to apply EXCEPT to %s, which is not a function.", pretty(f));
    if (f->kind == V_TUPLE && key->kind != V_INT) fail("Attempted to access tuple at a non integral index: %s", pretty(key));
    if (f->kind == V_FCN && f->made == MADE_RECORD && key->kind != V_STR) fail("Attempted to access record by a non-string argument: %s", pretty(key));
    int k = f->kind == V_LAMBDA ? -1 : fcn_index(f, key);
    if (f->kind != V_LAMBDA && k < 0) { warn_except(where->args[0]); return f; }
    Value *old = k >= 0 ? f->u.fcn.vals[k] : fcn_apply(f, key, where);
    Value *nv;
    if (i + 1 < p->n) {
        nv = except_update(old, p, i + 1, val, c, where);
    } else {
        Value *saved = at_value;
        at_value = old;
        nv = eval(val, c);
        at_value = saved;
    }
    return fcn_update(f, k, key, nv);
}

static int eval_bool(Expr *e, Ctx c) {
    Value *v = eval(e, c);
    if (v->kind != V_BOOL)
        fail_at(e, "A non-boolean expression (%s) was used as a condition.", kind_name(v));
    return (int)v->u.i;
}

static const char *article_kind(Value *v) {
    switch (v->kind) {
    case V_INT: return "an integer";
    case V_STR: return "a string";
    case V_MODEL: return "a model value";
    case V_TUPLE: case V_FCN: return "a function";
    default: return is_set(v) ? "a set" : "a value";
    }
}

static Value *junction(Expr *e, Ctx c, int and) {
    for (int i = 0; i < e->n; i++) {
        Value *v = eval(e->args[i], c);
        if (v->kind != V_BOOL)
            fail_at(e, "Attempted to evaluate an expression of form %s when %s was\n%s.",
                    and ? "P /\\ Q" : "P \\/ Q", i == 0 ? "P" : "Q", article_kind(v));
        if (v->u.i != and) return v;
    }
    return BOOLV(and);
}

/* The instances an instance call goes through, each with its parameters. */
static Inst *instance_chain(Expr *e, Ctx c) {
    Inst *inst = c.inst;
    for (int i = 0; i < e->ndefs; i++) {
        Def *id = e->defs[i];
        Env *env = NULL;
        Seg *s = &e->segs[i];
        for (int k = 0; k < id->nparams && k < s->n; k++) env = mk_thunk(id->params[k], s->args[k], c, env);
        if (id->local == 2) { Env *b = env_find(c.env, id); if (b) env = b->ctx_env; }
        Inst *in = NEW(Inst);
        in->def = id;
        in->env = env;
        in->parent = inst;
        inst = in;
    }
    return inst;
}



static __attribute__((noinline)) Value *ev_bound(Expr *e, Ctx c) {
    Env *b = env_find(c.env, e->def);
    if (!b) fail_runtime(e, TPP_ERROR_CONFIG, "In evaluation, the identifier %s is either undefined or not an operator.", S(e->def->name));
    return force(b);
}

static __attribute__((noinline)) Value *ev_call(Expr *e, Ctx c) {
    Def *d = e->def;
    if (d->kind == D_VAR && !d->replacement) return decl_value(d, c, e);
    if (d->native && !d->replacement && e->n <= 4 && e->n == d->nparams) {
        /* The common operators of the standard modules: no arena for the arguments. */
        Value *vals[4];
        int plain = 1;
        for (int i = 0; i < e->n && plain; i++) plain = !d->params[i]->arity;
        if (plain) {
            for (int i = 0; i < e->n; i++) vals[i] = eval(e->args[i], c);
            return call_native(d, e, vals, c);
        }
    }
    return call_def(d, e->args, e->n, c, e);
}

static __attribute__((noinline)) Value *ev_instcall(Expr *e, Ctx c) {
    Inst *inst = instance_chain(e, c);
    Ctx ic = {c.env, inst};
    /* The arguments are evaluated in the caller's context. */
    Value **vals = NEWN(Value *, e->n);
    Def *d = e->def;
    if (d->replacement) return call_def(d, e->args, e->n, c, e);   /* by the configuration, an operator of the root module */
    if (d->kind == D_CONST || d->kind == D_VAR) return decl_value(d, ic, e);
    if (d->native || d->kind == D_FCN || e->n == 0) {
        if (e->n == 0) { Ctx bare = {NULL, inst}; return call_def(d, NULL, 0, bare, e); }
    }
    Env *env = NULL;
    for (int k = 0; k < e->n && k < d->nparams; k++) {
        if (d->params[k]->arity) env = mk_binding(d->params[k], eval_operator_arg(e->args[k], c), env);
        else env = mk_thunk(d->params[k], e->args[k], c, env);
    }
    if (d->native) {
        for (int k = 0; k < e->n; k++) vals[k] = force(env_find(env, d->params[k]));
        return call_native(d, e, vals, ic);
    }
    Ctx body = {env, inst};
    return eval_body(d->body, body);
}

static __attribute__((noinline)) Value *ev_paramcall(Expr *e, Ctx c) {
    Value *op;
    if (e->def->kind == D_PARAM) {
        Env *b = env_find(c.env, e->def);
        if (!b) fail_at(e, "Unbound operator parameter %s.", S(e->def->name));
        op = force(b);
    } else {
        op = decl_value(e->def, c, e);
    }
    if (op->kind != V_OPER) {
        if (e->n == 0) return op;
        fail_at(e, "%s is not an operator.", S(e->def->name));
    }
    Value **vals = NEWN(Value *, e->n);
    for (int i = 0; i < e->n; i++) vals[i] = eval(e->args[i], c);
    return apply_oper(op, vals, e->n, e);
}

static __attribute__((noinline)) Value *ev_not(Expr *e, Ctx c) {
    Value *v = eval(e->args[0], c);
    if (v->kind != V_BOOL) fail_at(e, "Attempted to apply the operator ~ to a non-boolean\n(%s).", pretty(v));
    return BOOLV(!v->u.i);
}

static __attribute__((noinline)) Value *ev_implies(Expr *e, Ctx c) {
    Value *a = eval(e->args[0], c);
    if (a->kind != V_BOOL) fail_at(e, "Attempted to evaluate an expression of form P => Q when P was\n%s.", article_kind(a));
    if (!a->u.i) return TRUEV;
    Value *b = eval(e->args[1], c);
    if (b->kind != V_BOOL) fail_at(e, "Attempted to evaluate an expression of form P => Q when Q was\n%s.", article_kind(b));
    return b;
}

static __attribute__((noinline)) Value *ev_equiv(Expr *e, Ctx c) {
    Value *a = eval(e->args[0], c), *b = eval(e->args[1], c);
    if (a->kind != V_BOOL || b->kind != V_BOOL) fail_at(e, "Attempted to evaluate an expression of form P <=> Q when P or Q was not a boolean.");
    return BOOLV(a->u.i == b->u.i);
}

/* The left side first, as TLC: C leaves the order of arguments open. */
static __attribute__((noinline)) Value *ev_eq(Expr *e, Ctx c) {
    Value *a = eval(e->args[0], c), *b = eval(e->args[1], c);
    return BOOLV(veq(a, b));
}

static __attribute__((noinline)) Value *ev_neq(Expr *e, Ctx c) {
    Value *a = eval(e->args[0], c), *b = eval(e->args[1], c);
    return BOOLV(!veq(a, b));
}

static __attribute__((noinline)) Value *ev_in(Expr *e, Ctx c) {
    Value *v = eval(e->args[0], c), *s = eval(e->args[1], c);
    int in = set_member(v, s);
    return BOOLV(e->kind == K_IN ? in : !in);
}

static __attribute__((noinline)) Value *ev_cup(Expr *e, Ctx c) {
    Value *a = eval(e->args[0], c), *b = eval(e->args[1], c);
    if (e->kind == K_SUBSETEQ) {
        if (!is_set(a) || user_set(a)) fail_at(e, "Attempted to evaluate an expression of form S \\subseteq T, but S was not enumerable.");
        return BOOLV(all_members(a, b));
    }
    Value *r = e->kind == K_CUP ? set_union(a, b) : e->kind == K_CAP ? set_inter(a, b) : set_minus(a, b);
    if (r != a && r != b && r->kind == V_LAZY && r->lazy >= L_CUP) r->u.l.expr = source(e);
    return r;
}

static Value *subset_of(Expr *e, Ctx c) {
    Value *s = mk_lazy(L_SUBSET);
    s->u.l.expr = source(e);
    s->u.l.a = eval(e->args[0], c);
    if (!is_set(s->u.l.a)) fail_at(e, "Attempted to compute SUBSET of the non-set:\n%s", pretty(s->u.l.a));
    return s;
}

static __attribute__((noinline)) Value *ev_union(Expr *e, Ctx c) {
    Expr *x = e->args[0];
    Value *s;
    int hold = 0;   /* TLC combines only sets of their elements; it keeps another UNION as it is */
    if (x->kind == X_SETENUM) {
        /* TLC sorts a set's elements only once it must, as Nat and {-1}
           cannot be: UNION {{-1}, Nat} is a set. */
        s = mk_set(x->n);
        for (int i = 0; i < x->n; i++) s->u.set.elems[i] = eval(x->args[i], c);
        s->u.set.source = source(x);
    } else {
        s = eval(x, c);
        hold = s->kind != V_SET || s->made == MADE_HELD;
        s = enumerate(s);
    }
    for (int i = 0; i < s->n; i++) hold |= s->u.set.elems[i]->kind != V_SET || s->u.set.elems[i]->made == MADE_HELD;
    if (hold) {
        Value *u = mk_lazy(L_UNION);
        u->u.l.a = s;
        u->u.l.expr = source(e);
        u->n = x->kind == X_SETENUM;   /* its sets, to be sorted */
        return u;
    }
    if (x->kind == X_SETENUM) s = set_normalize(s);
    List l = {0};
    for (int i = 0; i < s->n; i++)
        for (int j = 0; j < s->u.set.elems[i]->n; j++) list_add(&l, s->u.set.elems[i]->u.set.elems[j]);
    return list_set(&l);
}

static __attribute__((noinline)) Value *ev_domain(Expr *e, Ctx c) {
    Value *f = eval(e->args[0], c);
    if (!is_fcn(f) && f->kind != V_LAMBDA) fail_at(e, "Attempted to apply the operator DOMAIN to a non-function\n(%s).", kind_string(f));
    return fcn_domain(f);
}

static __attribute__((noinline)) Value *ev_prime(Expr *e, Ctx c) {
    if (primed) return eval(e->args[0], c);
    primed = 1;
    Value *v = eval(e->args[0], c);
    primed = 0;
    return v;
}

/* Whether e' = e. */
static int kept(Expr *e, Ctx c) {
    Value *now = eval(e, c);
    int saved = primed;
    primed = 1;
    Value *then = eval(e, c);
    primed = saved;
    return veq(now, then);
}

static __attribute__((noinline)) Value *ev_unchanged(Expr *e, Ctx c) { return BOOLV(kept(e->args[0], c)); }

/* Whether e has one value wherever it is evaluated outside an instance:
   it is made of literals, constants, constant definitions and the
   operators of the arithmetic, sequence and set modules. */
static int fixed_expr(Expr *e) {
    if (e->fixed) return e->fixed == 1;
    Def *d = e->def && e->def->replacement ? e->def->replacement : e->def;
    int f = 0;
    switch (e->kind) {
    case X_NUM: case X_STR: case X_BOOL: f = 1; break;
    case K_DECL: f = d->kind == D_CONST; break;
    case X_SETENUM: case X_TUPLE: case K_TIMES: case K_SUBSET: case X_FCNSET: case X_RCDSET: f = 1; break;
    case K_CALL:
        if (d->native) {
            const char *m = S(d->mod->name);
            f = !strcmp(m, "Naturals") || !strcmp(m, "Integers") || !strcmp(m, "Sequences") || !strcmp(m, "FiniteSets");
        } else {
            f = d->kind == D_OP && !d->nparams && d->local != 2 && d->cache;
        }
        break;
    }
    for (int i = 0; i < e->n && f; i++) f = fixed_expr(e->args[i]);
    e->fixed = f ? 1 : 2;
    return f;
}

/* The set make makes of e: in the search, where e is fixed, once, in the
   permanent arena, naming e as its source as a set made anew would. */
static Value *ev_fixed(Expr *e, Ctx c, Value *(*make)(Expr *, Ctx)) {
    if (c.inst || arena != &scratch_arena) return make(e, c);
    if (e->value) return e->value;
    if (!fixed_expr(e)) return make(e, c);
    Arena *saved = arena;
    sigjmp_buf *outer = handler, here;
    handler = &here;
    if (sigsetjmp(here, 0)) { handler = outer; arena = saved; raise_error(); }
    arena = &perm_arena;
    Value *t = make(e, c);
    (t->kind == V_LAZY ? t : t->u.set.product)->u.l.expr = e;
    handler = outer;
    arena = saved;
    return e->value = t;
}

static Value *product(Expr *e, Ctx c);
static Value *rcdset_of(Expr *e, Ctx c);
static Value *fcnset_of(Expr *e, Ctx c);

static __attribute__((noinline)) Value *ev_times(Expr *e, Ctx c) { return ev_fixed(e, c, product); }
static __attribute__((noinline)) Value *ev_subset(Expr *e, Ctx c) { return ev_fixed(e, c, subset_of); }
static __attribute__((noinline)) Value *ev_rcdset(Expr *e, Ctx c) { return ev_fixed(e, c, rcdset_of); }
static __attribute__((noinline)) Value *ev_fcnset(Expr *e, Ctx c) { return ev_fixed(e, c, fcnset_of); }

static Value *product(Expr *e, Ctx c) {
    Value *s = mk_lazy(L_TIMES);
    s->u.l.expr = source(e);
    s->n = e->n;
    s->u.l.names = NEWN(Value *, 2 * e->n);
    for (int i = 0; i < e->n; i++) {
        Value *a = eval(e->args[i], c);
        if (!is_set(a)) fail_at(e, "Attempted to compute the cartesian product with the non-set:\n%s", pretty(a));
        s->u.l.names[e->n + i] = a;
    }
    fp_enter(s->u.l.expr);   /* sizing its parts, as TLC enumerating it */
    int expand = expandable(s);
    fp_leave(s->u.l.expr);
    if (!expand) return s;
    Value *t = held(enumerate(s));
    t->u.set.product = s;   /* as TLC, RandomElement draws each part */
    return t;
}

static __attribute__((noinline)) Value *ev_setenum(Expr *e, Ctx c) {
    if (e->value) return e->value;
    int literal = e->n > 0;
    for (int i = 0; i < e->n && literal; i++)
        literal = e->args[i]->kind == e->args[0]->kind && (e->args[i]->kind == X_STR || e->args[i]->kind == X_NUM);
    if (literal) {   /* its elements compare without error */
        Arena *saved = arena;
        arena = &perm_arena;
        List l = {0};
        for (int i = 0; i < e->n; i++) list_add(&l, eval(e->args[i], c));
        e->value = list_set(&l);
        e->value->u.set.source = e;
        arena = saved;
        return e->value;
    }
    List l = {0};
    for (int i = 0; i < e->n; i++) list_add(&l, eval(e->args[i], c));
    fp_enter(e);   /* comparing its elements */
    Value *s = list_set(&l);
    fp_leave(e);
    s->u.set.source = source(e);
    return s;
}

static __attribute__((noinline)) Value *ev_tuple(Expr *e, Ctx c) {
    Value *t = mk_tuple(e->n);
    for (int i = 0; i < e->n; i++) t->u.fcn.vals[i] = eval(e->args[i], c);
    t->u.fcn.source = source(e);
    return t;
}

static __attribute__((noinline)) Value *ev_record(Expr *e, Ctx c) {
    List keys = {0}, vals = {0};
    for (int i = 0; i < e->n; i++) { list_add(&keys, mk_str(e->segs[i].name)); list_add(&vals, eval(e->args[i], c)); }
    Value *r = mk_fcn_from(&keys, &vals);
    r->made = MADE_RECORD;
    r->u.fcn.source = source(e);
    return r;
}

static Value *rcdset_of(Expr *e, Ctx c) {
    List keys = {0}, vals = {0};
    for (int i = 0; i < e->n; i++) {
        Value *s = eval(e->args[i], c);
        if (!is_set(s)) fail_at(e, "Attempted to construct a set of records with the non-set:\n%s", pretty(s));
        list_add(&keys, mk_str(e->segs[i].name));
        list_add(&vals, s);
    }
    Value *r = mk_fcn_from(&keys, &vals);
    Value *s = mk_lazy(L_RCDSET);
    s->u.l.expr = source(e);
    s->n = e->n;
    s->u.l.names = NEWN(Value *, 2 * e->n);
    for (int i = 0; i < e->n; i++) { s->u.l.names[i] = r->u.fcn.keys[i]; s->u.l.names[e->n + i] = r->u.fcn.vals[i]; }
    return s;
}

static Value *fcnset_of(Expr *e, Ctx c) {
    Value *s = mk_lazy(L_FCNSET);
    s->u.l.expr = source(e);
    s->u.l.a = eval(e->args[0], c);
    s->u.l.b = eval(e->args[1], c);
    if (!is_set(s->u.l.a) || !is_set(s->u.l.b)) fail_at(e, "Attempted to construct a set of functions with a non-set.");
    return s;
}

static __attribute__((noinline)) Value *ev_fcn(Expr *e, Ctx c) {
    Value *dom = bounds_domain(e->bounds, e->nb, c);
    if (keep_lazy || !bounds_reducible(dom, bounds_total(e->bounds, e->nb))) return mk_lambda(dom, e, NULL, c);
    return fcn_in_full(dom, e, NULL, c, e);
}

/* The function f[a] applies, evaluated lazily if lazy, and its argument
   *key; NULL for a function definition, applied without its value. */
static Value *apply_parts(Expr *e, Ctx c, int lazy, Value **key) {
    Expr *fe = e->args[0];
    Value *f = NULL;
    if (!(lazy && fe->kind == K_CALL && fe->def->kind == D_FCN && !fe->def->replacement && !fe->def->cache)) {
        int keep = keep_lazy;
        keep_lazy = lazy;
        f = eval(fe, c);
        keep_lazy = keep;
    }
    if (e->n == 2) *key = eval(e->args[1], c);
    else {
        *key = mk_tuple(e->n - 1);
        for (int i = 1; i < e->n; i++) (*key)->u.fcn.vals[i - 1] = eval(e->args[i], c);
    }
    if (f && !is_fcn(f) && f->kind != V_LAMBDA) fail_at(e, "A non-function (%s) was applied as a function.", kind_string(f));
    return f;
}

static __attribute__((noinline)) Value *ev_apply(Expr *e, Ctx c) {
    Value *key, *f = apply_parts(e, c, 1, &key);   /* a function to apply stays lazy (§6.6) */
    return f ? fcn_apply(f, key, e) : fcndef_apply(e->args[0]->def, key, c);
}

static __attribute__((noinline)) Value *ev_dot(Expr *e, Ctx c) {
    Value *r = eval(e->args[0], c);
    if (!is_fcn(r)) fail_at(e, "Attempted to select field \"%s\" from a non-record value %s", S(e->op), pretty(r));
    Value *field = mk_str(e->op);
    if (r->made == MADE_RECORD) {
        int i = fcn_index(r, field);
        if (i < 0) fail_at(e, "Attempted to select nonexistent field \"%s\" from the record\n%s", S(e->op), pretty(r));
        return r->u.fcn.vals[i];
    }
    if (r->kind == V_TUPLE) fail("Attempted to apply function with integer domain to the non-integer argument %s", pretty(field));
    /* TLC compares each element of the domain with the field, in order. */
    for (int i = 0; i < r->n; i++) if (veq(r->u.fcn.keys[i], field)) return r->u.fcn.vals[i];
    fail("Attempted to apply function:\n%s\nto argument %s, which is not in the domain of the function.",
         pretty(r), pretty(field));
}

static __attribute__((noinline)) Value *ev_except(Expr *e, Ctx c) {
    Value *f = eval(e->args[0], c);
    for (int i = 0; i < e->n - 1; i++) f = except_update(f, &e->paths[i], 0, e->args[i + 1], c, e);
    return f;
}

static __attribute__((noinline)) Value *ev_filter(Expr *e, Ctx c) {
    Bound *b = &e->bounds[0];
    Value *s = eval(b->set, c);
    Value *l = mk_lazy(L_FILTER);
    l->u.l.a = s;
    l->u.l.expr = e;
    l->u.l.env = c.env;
    l->u.l.inst = c.inst;
    l->u.l.made_in = made_in();
    /* As in TLC, a filter of a set of its elements or an interval is
       computed at once; another is lazy, membership evaluating the
       predicate on that element only. */
    return reducible(s) ? enumerate(l) : l;
}

static __attribute__((noinline)) Value *ev_map(Expr *e, Ctx c) {
    MapData d;
    memset(&d, 0, sizeof d);
    d.body = e->args[0];
    for_bindings(e->bounds, e->nb, c, map_fn, &d);
    return list_set(&d.out);
}

static __attribute__((noinline)) Value *ev_forall(Expr *e, Ctx c) {
    QuantData q = {e->args[0], e->kind == X_FORALL, e};
    const char *op = e->kind == X_FORALL ? "\\A" : "\\E";
    for (int i = 0; i < e->nb; i++)
        if (!e->bounds[i].set)
            fail_at(e, "TLC attempted to evaluate an unbounded %s.\nMake sure that the expression is of form %s x \\in S: P(x).", op, op);
    for_bindings(e->bounds, e->nb, c, e->kind == X_FORALL ? forall_fn : exists_fn, &q);
    return BOOLV(q.value);
}

static __attribute__((noinline)) Value *ev_choose(Expr *e, Ctx c) {
    Bound *b = &e->bounds[0];
    if (!b->set) fail_at(e, "TLC attempted to evaluate an unbounded CHOOSE.\nMake sure that the expression is of form CHOOSE x \\in S: P(x).");
    Value *s = eval(b->set, c);
    if (!is_set(s) || user_set(s))
        fail_at(e, "Attempted to compute the value of an expression of\nform CHOOSE x \\in S: P, but S was not enumerable.");
    s = enumerate(s);
    for (int i = 0; i < s->n; i++) {
        Value *x = s->u.set.elems[i];
        Env *env = c.env;
        if (!b->tuple) {
            env = mk_binding(b->defs[0], x, env);
        } else {
            if (!is_fcn(x) || !fcn_is_tuple(x) || x->n != b->n)
                fail_at(e, "Attempted to compute the value of an expression of form\n"
                           "CHOOSE <<x1, ... , xN>> \\in S: P, but S was not a set\nof N-tuples.");
            for (int j = 0; j < b->n; j++) env = mk_binding(b->defs[j], x->u.fcn.vals[j], env);
        }
        if (eval_bool(e->args[0], (Ctx){env, c.inst})) return x;
    }
    fail_at(e, "Attempted to compute the value of an expression of form\nCHOOSE x \\in S: P, but no element of S satisfied P.");
}

static __attribute__((noinline)) Value *ev_if(Expr *e, Ctx c) {
    Value *v = eval(e->args[0], c);
    if (v->kind != V_BOOL) fail_at(e, "A non-boolean expression (%s) was used as the condition of an IF.", kind_string(v));
    return eval(e->args[v->u.i ? 1 : 2], c);
}

static __attribute__((noinline)) Value *ev_case(Expr *e, Ctx c) {
    for (int i = 0; i + 1 < e->n; i += 2) {
        if (!e->args[i]) return eval(e->args[i + 1], c);
        if (eval_bool(e->args[i], c)) return eval(e->args[i + 1], c);
    }
    fail_at(e, "Attempted to evaluate a CASE with no conditions true.");
}

static __attribute__((noinline)) Value *ev_let(Expr *e, Ctx c) {
    Env *env = c.env;
    for (int i = 0; i < e->ndefs; i++) env = mk_thunk(e->defs[i], e->defs[i]->body, c, env);
    /* The definitions see each other: close them over the new environment. */
    for (Env *b = env; b != c.env; b = b->next) b->ctx_env = env;
    Ctx inner = {env, c.inst};
    return eval(e->args[0], inner);
}

static __attribute__((noinline)) Value *ev_sqact(Expr *e, Ctx c) {
    /* [A]_v and <<A>>_v evaluated on a pair of states; as TLC, v only
       when A does not decide. */
    int sq = e->kind == X_SQACT;
    Value *a = eval(e->args[0], c);
    if (a->kind != V_BOOL) fail_at(e, "Attempted to evaluate an expression of form %s, but A was not a boolean.", sq ? "[A]_e" : "<A>_e");
    if (a->u.i == sq) return a;
    Value *now = eval(e->args[1], c);
    int saved = primed;
    primed = 1;
    Value *then = eval(e->args[1], c);
    primed = saved;
    return BOOLV(veq(now, then) == sq);
}

#define EVAL(name) static Value *name(Expr *e, Ctx c)
#define UNUSED_C (void)c
EVAL(ev_num) { UNUSED_C; return mk_int(e->num); }   /* larger ones are rejected before checking */
EVAL(ev_str) { UNUSED_C; return mk_str((int)e->num); }
EVAL(ev_bool) { UNUSED_C; return BOOLV(e->num); }
EVAL(ev_boolean) { UNUSED_C; (void)e; return boolean_set; }
EVAL(ev_stringset) { UNUSED_C; (void)e; return mk_lazy(L_STRING); }
EVAL(ev_at) {
    UNUSED_C;
    if (!at_value) fail_at(e, "@ used outside an EXCEPT.");
    return at_value;
}
EVAL(ev_decl) { return decl_value(e->def, c, e); }
EVAL(ev_and) { return junction(e, c, 1); }
EVAL(ev_or) { return junction(e, c, 0); }
EVAL(ev_enabled) { return BOOLV(enabled(e->args[0], c)); }
EVAL(ev_lambda) { return mk_oper(e->def, c); }
EVAL(ev_opref) { return mk_oper(e->def, (Ctx){NULL, c.inst}); }
EVAL(ev_temporal) {
    UNUSED_C;
    const char *what = e->kind == K_ALWAYS ? "[]A" : e->kind == K_EVENTUALLY ? "<>A" : e->kind == K_LEADSTO ? "a ~> b"
                     : e->kind == K_WHILE ? "a -+-> formula" : e->kind == X_WF ? "WF" : e->kind == X_SF ? "SF"
                     : e->kind == X_TFORALL ? "\\AA" : "\\EE";
    fail_runtime(e, TPP_ERROR, "TLC encountered a temporal formula (%s) when evaluating a predicate or action.", what);
}
EVAL(ev_cdot) { UNUSED_C; (void)e; fail("The current version of TLC does not support action composition.  An incomplete implementation can be enabled via the tlc2.tool.impl.Tool.cdot=true java property."); }
#undef UNUSED_C
#undef EVAL

/* The evaluator of each kind of expression (eval). */
static const EvalFn eval_kind[K_WHILE + 1] = {
    [X_NUM] = ev_num,
    [X_STR] = ev_str,
    [X_BOOL] = ev_bool,
    [X_NAME] = ev_temporal,
    [X_OP] = ev_temporal,
    [X_AT] = ev_at,
    [X_LAMBDA] = ev_lambda,
    [X_OPREF] = ev_opref,
    [X_SETENUM] = ev_setenum,
    [X_TUPLE] = ev_tuple,
    [X_RECORD] = ev_record,
    [X_RCDSET] = ev_rcdset,
    [X_FCNSET] = ev_fcnset,
    [X_FCN] = ev_fcn,
    [X_APPLY] = ev_apply,
    [X_DOT] = ev_dot,
    [X_EXCEPT] = ev_except,
    [X_FILTER] = ev_filter,
    [X_MAP] = ev_map,
    [X_FORALL] = ev_forall,
    [X_EXISTS] = ev_forall,
    [X_CHOOSE] = ev_choose,
    [X_IF] = ev_if,
    [X_CASE] = ev_case,
    [X_LET] = ev_let,
    [X_SQACT] = ev_sqact,
    [X_ANGLEACT] = ev_sqact,
    [X_WF] = ev_temporal,
    [X_SF] = ev_temporal,
    [X_TFORALL] = ev_temporal,
    [X_TEXISTS] = ev_temporal,
    [X_RESOLVED] = ev_temporal,
    [K_BOUND] = ev_bound,
    [K_DECL] = ev_decl,
    [K_CALL] = ev_call,
    [K_PARAMCALL] = ev_paramcall,
    [K_INSTCALL] = ev_instcall,
    [K_BOOLEAN] = ev_boolean,
    [K_STRINGSET] = ev_stringset,
    [K_AND] = ev_and,
    [K_OR] = ev_or,
    [K_NOT] = ev_not,
    [K_IMPLIES] = ev_implies,
    [K_EQUIV] = ev_equiv,
    [K_EQ] = ev_eq,
    [K_NEQ] = ev_neq,
    [K_IN] = ev_in,
    [K_NOTIN] = ev_in,
    [K_CUP] = ev_cup,
    [K_CAP] = ev_cup,
    [K_SETMINUS] = ev_cup,
    [K_SUBSETEQ] = ev_cup,
    [K_SUBSET] = ev_subset,
    [K_UNION] = ev_union,
    [K_DOMAIN] = ev_domain,
    [K_PRIME] = ev_prime,
    [K_UNCHANGED] = ev_unchanged,
    [K_ENABLED] = ev_enabled,
    [K_ALWAYS] = ev_temporal,
    [K_EVENTUALLY] = ev_temporal,
    [K_LEADSTO] = ev_temporal,
    [K_TIMES] = ev_times,
    [K_CDOT] = ev_cdot,
    [K_WHILE] = ev_temporal,
};

static inline Value *eval(Expr *e, Ctx c) { return eval_kind[e->kind](e, c); }


/* An argument for an operator parameter: an operator. */
static Value *eval_operator_arg(Expr *a, Ctx c) {
    if (a->kind == X_LAMBDA || a->kind == X_OPREF) return eval(a, c);
    if (a->kind == K_CALL && a->n == 0) return mk_oper(a->def, (Ctx){a->def->local == 2 ? c.env : NULL, c.inst});
    if (a->kind == K_PARAMCALL && a->n == 0 && a->def->kind == D_PARAM) {
        Env *b = env_find(c.env, a->def);
        if (!b) fail_at(a, "Unbound operator parameter %s.", S(a->def->name));
        return force(b);
    }
    if (a->kind == K_PARAMCALL && a->n == 0) return decl_value(a->def, c, a);
    if (a->kind == K_INSTCALL && a->n == 0) {
        Inst *inst = c.inst;
        for (int i = 0; i < a->ndefs; i++) {
            Inst *in = NEW(Inst);
            in->def = a->defs[i];
            in->parent = inst;
            inst = in;
        }
        return mk_oper(a->def, (Ctx){NULL, inst});
    }
    return eval(a, c);
}

static Value *call_values(Def *d, Value **vals, int n, Ctx c, Expr *where) {
    if (d->native) return call_native(d, where, vals, c);
    Env *env = c.env;
    if (d->kind == D_FCN) return fcndef_apply(d, n == 1 ? vals[0] : NULL, c);
    for (int i = 0; i < n && i < d->nparams; i++) env = mk_binding(d->params[i], vals[i], env);
    Ctx inner = {env, c.inst};
    return eval_body(d->body, inner);
}

static Value *apply_oper(Value *op, Value **args, int n, Expr *where) {
    Def *d = op->u.op.def;
    if (d->replacement) d = d->replacement;
    if (d->nparams != n && !d->native)
        fail_at(where, "The operator %s requires %d arguments.", S(d->name), d->nparams);
    Ctx c = {op->u.op.env, op->u.op.inst};
    if (d->local == 2 && !d->native) {
        Env *b = env_find(c.env, d);
        if (b) c.env = b->ctx_env;
    }
    return call_values(d, args, n, c, where);
}

/* TLC normalizes the value of a constant definition, as a state's. */
static void materialize(Value *v) {
    if (v->kind == V_LAZY) switch (v->lazy) {   /* a set TLC keeps: its parts */
    case L_UNION: v = union_sets(v); break;
    case L_CUP: case L_CAP: case L_DIFF: case L_FCNSET: materialize(v->u.l.b); /* fall through */
    case L_SUBSET: materialize(v->u.l.a); return;
    case L_TIMES: case L_RCDSET: for (int i = 0; i < v->n; i++) materialize(v->u.l.names[v->n + i]); return;
    }
    if (v->kind == V_SET) for (int i = 0; i < v->n; i++) materialize(v->u.set.elems[i]);
    if (!is_fcn(v)) return;
    if (v->made >= MADE_LAMBDA) v->made = MADE_PLAIN;
    for (int i = 0; i < v->n; i++) materialize(v->u.fcn.vals[i]);
}

/* v, made in another arena, as a value of this one; NULL past *budget
   values, or if it holds what keeps its context: an operator, a lazy
   function or a filter. A set keeps its sets, not what it cached. */
static Value **copy_values(Value **vs, int n, long *budget);

static Value *copy_value(Value *v, long *budget) {
    if (v->kind == V_BOOL || v->kind == V_STR) return v;
    if (--*budget < 0) return NULL;
    Value *c = NEW(Value);
    *c = *v;
    switch (v->kind) {
    case V_INT: case V_MODEL: case V_INTERVAL: return c;
    case V_SET:
        if (!(c->u.set.elems = copy_values(v->u.set.elems, v->n, budget))) return NULL;
        if (v->u.set.product && !(c->u.set.product = copy_value(v->u.set.product, budget))) return NULL;
        return c;
    case V_TUPLE: case V_FCN:
        if (v->kind == V_FCN && !(c->u.fcn.keys = copy_values(v->u.fcn.keys, v->n, budget))) return NULL;
        return (c->u.fcn.vals = copy_values(v->u.fcn.vals, v->n, budget)) ? c : NULL;
    case V_LAZY:
        c->perm = 1;
        c->u.l.b = c->u.l.set = NULL;   /* what it kept */
        switch (v->lazy) {
        case L_NAT: case L_INT: case L_STRING: case L_ANY: return c;
        case L_TIMES: case L_RCDSET: return (c->u.l.names = copy_values(v->u.l.names, 2 * v->n, budget)) ? c : NULL;
        case L_CUP: case L_CAP: case L_DIFF: case L_FCNSET: if (!(c->u.l.b = copy_value(v->u.l.b, budget))) return NULL; break;
        case L_UNION: case L_SEQ: case L_SUBSET: break;
        default: return NULL;
        }
        return (c->u.l.a = copy_value(v->u.l.a, budget)) ? c : NULL;
    default: return NULL;
    }
}

/* An array of values, any of them NULL, copied. */
static Value **copy_values(Value **vs, int n, long *budget) {
    Value **c = alloc_raw(sizeof(Value *) * (size_t)n);
    for (int i = 0; i < n; i++)
        if (!(c[i] = vs[i] ? copy_value(vs[i], budget) : NULL) && vs[i]) return NULL;
    return c;
}

/* ENABLED marks the instance chain where it is evaluated. As in TLC, a
   variable reached from there is not substituted (§6.9). */
static Def enabled_mark;

static Value *def_value(Def *d);

static Value *call_def(Def *d, Expr **args, int n, Ctx c, Expr *call) {
    if (d->replacement) d = d->replacement;
    if (d->kind == D_CONST || d->kind == D_VAR) return decl_value(d, c, call);
    if (d->value) return d->value;
    if (d->kind == D_FCN) {
        if (d->local != 2 && d->level == 0 && d->cache) return d->cache;
        Value *v = fcndef_value(d, c);
        return v;
    }
    if (d->kind == D_PARAM) {
        Env *b = env_find(c.env, d);
        return force(b);
    }
    if (n < d->nparams && n == 0) return mk_oper(d, (Ctx){d->local == 2 ? c.env : NULL, c.inst});
    if (d->local == 2) {
        /* A LET definition: its parameters, then its closure. */
        Env *b = env_find(c.env, d);
        if (!b) fail_at(call, "Unbound LET definition %s.", S(d->name));
        if (d->nparams == 0) return force(b);
        Env *env = b->ctx_env;
        for (int i = 0; i < n; i++) {
            if (d->params[i]->arity) env = mk_binding(d->params[i], eval_operator_arg(args[i], c), env);
            else env = mk_arg(d->params[i], args[i], c, env);
        }
        Ctx inner = {env, b->ctx_inst};
        return eval_body(d->body, inner);
    }
    if (d->native) {
        Value **vals = NEWN(Value *, n);
        for (int i = 0; i < n; i++)
            vals[i] = d->params[i]->arity ? eval_operator_arg(args[i], c) : eval(args[i], c);
        return call_native(d, call, vals, c);
    }
    Inst *in = c.inst;
    while (in && in->def == &enabled_mark) in = in->parent;
    if (n == 0 && !in) {   /* a definition of the spec, computed once if constant */
        if (d->cache) return d->cache;
        if (def_level(d) == 0 && !keep_lazy && !d->uncached) return d->cache = def_value(d);
    }
    Env *env = NULL;
    for (int i = 0; i < n; i++) {
        if (d->params[i]->arity) env = mk_binding(d->params[i], eval_operator_arg(args[i], c), env);
        else env = mk_arg(d->params[i], args[i], c, env);
    }
    Ctx inner = {env, c.inst};
    return eval_body(d->body, inner);
}

/* ---------------------------------------------------------------- standard modules */

#define NATIVE(name) static Value *name(Expr *call, Value **a, Ctx c)

static Value *call_native(Def *d, Expr *call, Value **args, Ctx c) {
    if (java_depth < 64) java_frames[java_depth] = d->java;
    java_depth++;
    Value *v = d->native(call, args, c);
    java_depth--;
    return v;
}
#define UNUSED (void)call; (void)a; (void)c

NATIVE(n_nat) { UNUSED; return mk_lazy(L_NAT); }
NATIVE(n_int) { UNUSED; return mk_lazy(L_INT); }
NATIVE(n_plus) { UNUSED; int64_t x = int_cast(a[0]), y = int_cast(a[1]); return mk_int(check_int(x + y, x, y, "+")); }
NATIVE(n_minus) { UNUSED; int64_t x = int_cast(a[0]), y = int_cast(a[1]); return mk_int(check_int(x - y, x, y, "-")); }
NATIVE(n_times) { UNUSED; int64_t x = int_cast(a[0]), y = int_cast(a[1]); return mk_int(check_int(x * y, x, y, "*")); }
NATIVE(n_lt) { UNUSED; int64_t x = int_arg(a[0], 1, "<"), y = int_arg(a[1], 2, "<"); return BOOLV(x < y); }
NATIVE(n_gt) { UNUSED; int64_t x = int_arg(a[0], 1, ">"), y = int_arg(a[1], 2, ">"); return BOOLV(x > y); }
NATIVE(n_leq) { UNUSED; int64_t x = int_arg(a[0], 1, "<="), y = int_arg(a[1], 2, "<="); return BOOLV(x <= y); }
NATIVE(n_geq) { UNUSED; int64_t x = int_arg(a[0], 1, ">="), y = int_arg(a[1], 2, ">="); return BOOLV(x >= y); }
NATIVE(n_neg) { UNUSED; int64_t x = int_cast(a[0]); return mk_int(check_int(-x, 0, x, "-")); }

NATIVE(n_pow) {
    UNUSED;
    int64_t x = int_cast(a[0]), y = int_cast(a[1]);
    if (y < 0) fail_module("The second argument of ^ should be a natural number, but instead it is:\n%lld", (long long)y);
    if (x == 0 && y == 0) fail_module("0^0 is undefined.");
    int64_t r = 1;
    for (int64_t i = 0; i < y; i++) {
        r *= x;
        if (r > 2147483647LL || r < -2147483648LL) fail_module("Overflow when computing %lld^%lld", (long long)x, (long long)y);
    }
    return mk_int(r);
}

NATIVE(n_div) {
    UNUSED;
    int64_t x = int_cast(a[0]), y = int_cast(a[1]);
    if (y == 0) fail_module("The second argument of \\div is 0.");
    int64_t q = x / y;
    if ((x % y != 0) && ((x < 0) != (y < 0))) q--;
    return mk_int(check_int(q, x, y, "\\div"));
}

NATIVE(n_mod) {
    UNUSED;
    int64_t x = int_cast(a[0]), y = int_cast(a[1]);
    if (y <= 0) fail_module("The second argument of %% should be a positive number, but instead it is:\n%lld", (long long)y);
    int64_t r = x % y;
    if (r < 0) r += y;
    return mk_int(r);
}

NATIVE(n_range) {
    UNUSED;
    return mk_interval(int_cast(a[0]), int_cast(a[1]));
}

/* A sequence: a function on 1..n, or a string. */
static Value *seq_arg(Value *v, const char *error) {
    v = realize(v);
    if (is_fcn(v) && (fcn_is_tuple(v) || v->n == 0)) return v;
    if (v->kind == V_STR) return str_chars(v);
    fail_module("%s:\n%s", error, pretty(v));
}

#define NOT_SEQ(arg, op) "The " arg " of " op " should be a sequence, but instead it is"

static Value *seq_of(Value *f) {
    if (f->kind == V_TUPLE) return f;
    Value *t = mk_tuple(f->n);
    for (int i = 0; i < f->n; i++) t->u.fcn.vals[i] = f->u.fcn.vals[i];
    return t;
}

NATIVE(n_seq) { UNUSED; Value *s = mk_lazy(L_SEQ); s->u.l.a = a[0]; return s; }

NATIVE(n_len) {
    UNUSED;
    if (a[0]->kind == V_STR) return mk_int(strs[a[0]->u.i].len);
    return mk_int(seq_arg(a[0], NOT_SEQ("argument", "Len"))->n);
}

/* s \o t for a string s. */
static Value *str_concat(Value *s, Value *t) {
    if (t->kind != V_STR) fail_module("Evaluating an expression of the form t \\o s when s is not a string:\n%s", pretty(t));
    Buf b = {0};
    buf_puts(&b, S(s->u.i));
    buf_puts(&b, S(t->u.i));
    int id = intern_n(b.s ? b.s : "", (int)b.n);
    free(b.s);
    return mk_str(id);
}

NATIVE(n_concat) {
    UNUSED;
    if (a[0]->kind == V_STR) return str_concat(a[0], a[1]);
    Value *x = seq_arg(a[0], "Evaluating an expression of the form s \\o t when s is not a sequence"),
          *y = seq_arg(a[1], "Evaluating an expression of the form t \\o s when s is not a sequence");
    Value *t = mk_tuple(x->n + y->n);
    for (int i = 0; i < x->n; i++) t->u.fcn.vals[i] = x->u.fcn.vals[i];
    for (int i = 0; i < y->n; i++) t->u.fcn.vals[x->n + i] = y->u.fcn.vals[i];
    return t;
}

NATIVE(n_append) {
    UNUSED;
    if (a[0]->kind == V_STR) return str_concat(a[0], a[1]);
    Value *x = seq_arg(a[0], "Evaluating an expression of the form Append(s, v) when s is not a sequence");
    Value *t = mk_tuple(x->n + 1);
    for (int i = 0; i < x->n; i++) t->u.fcn.vals[i] = x->u.fcn.vals[i];
    t->u.fcn.vals[x->n] = a[1];
    if (x->u.fcn.hash) t->u.fcn.hash = x->u.fcn.hash + pair_hash(t, x->n, a[1]);
    return t;
}

NATIVE(n_head) {
    UNUSED;
    Value *x = seq_arg(a[0], NOT_SEQ("argument", "Head"));
    if (x->n == 0) fail_module("Attempted to apply Head to the empty sequence.");
    return x->u.fcn.vals[0];
}

NATIVE(n_tail) {
    UNUSED;
    Value *x = seq_arg(a[0], NOT_SEQ("argument", "Tail"));
    if (x->n == 0) fail_module("Attempted to apply Tail to the empty sequence.");
    if (a[0]->kind == V_STR) return substr(a[0], 1, x->n - 1);
    Value *t = mk_tuple(x->n - 1);
    for (int i = 1; i < x->n; i++) t->u.fcn.vals[i - 1] = x->u.fcn.vals[i];
    return t;
}

NATIVE(n_subseq) {
    UNUSED;
    Value *x = seq_arg(a[0], NOT_SEQ("first argument", "SubSeq"));
    for (int i = 1; i < 3; i++)
        if (a[i]->kind != V_INT)
            fail_module("The %s argument of SubSeq should be a natural number, but instead it is:\n%s", i == 1 ? "second" : "third", pretty(a[i]));
    int64_t m = a[1]->u.i, n = a[2]->u.i;
    int str = a[0]->kind == V_STR;
    if (m > n) return str ? substr(a[0], 0, 0) : mk_tuple(0);
    int second = m < 1 || m > x->n;
    if (second || n > x->n)
        fail_module("The %s argument of SubSeq must be in the domain of its first argument:\n%s\n, but instead it is\n%lld",
                    second ? "second" : "third", pretty(a[0]), (long long)(second ? m : n));
    if (str) return substr(a[0], (int)m - 1, (int)(n - m + 1));
    Value *t = mk_tuple((int)(n - m + 1));
    for (int64_t i = m; i <= n; i++) t->u.fcn.vals[i - m] = x->u.fcn.vals[i - 1];
    return t;
}

NATIVE(n_selectseq) {
    UNUSED;
    Value *x = seq_arg(a[0], NOT_SEQ("first argument", "SelectSeq"));
    List l = {0};
    for (int i = 0; i < x->n; i++) {
        Value *r = apply_oper(a[1], &x->u.fcn.vals[i], 1, call);
        if (r->kind != V_BOOL) fail_module("The second argument of SelectSeq should be a operator, but instead it is:\n%s", pretty(r));
        if (r->u.i) list_add(&l, x->u.fcn.vals[i]);
    }
    Value *t = mk_tuple(l.n);
    for (int i = 0; i < l.n; i++) t->u.fcn.vals[i] = l.v[i];
    return t;
}

/* TLC's isFinite, which IsFiniteSet asks: a function is a finite set,
   another value an error, and some sets cannot tell. */
static int tlc_finite(Value *s) {
    static const char *what[] = {[V_BOOL] = "boolean", [V_INT] = "integer", [V_STR] = "string", [V_MODEL] = "model value"};
    Value *a = s->u.l.a, *b = s->u.l.b;
    switch (s->kind) {
    case V_BOOL: case V_INT: case V_STR: case V_MODEL:
        fail("Attempted to check if the %s %s is a finite set.", what[s->kind], pretty(s));
    case V_LAMBDA: fail("Attempted to check if the function:\n%s\nis a finite set.", pretty(s));
    case V_LAZY: break;
    default: return 1;
    }
    switch (s->lazy) {
    case L_FILTER:
        if (!tlc_finite(a))
            fail("Attempted to check if expression of form {x \\in S : p(x)} is a finite set, but cannot check if S:\n%s\nis finite.", pretty(a));
        return 1;
    case L_CAP: case L_DIFF:
        if (s->lazy == L_DIFF ? tlc_finite(a) : tlc_finite(a) || tlc_finite(b)) return 1;
        if (s->lazy == L_CAP || !tlc_finite(b)) fail("Attempted to check if the set %sis finite.", pretty(s));
        return 0;
    case L_CUP: return tlc_finite(a) && tlc_finite(b);
    case L_SUBSET: return tlc_finite(a);
    case L_UNION:
        union_sets(s);
        for (int i = 0; i < s->u.l.a->n; i++) if (!tlc_finite(s->u.l.a->u.set.elems[i])) return 0;
        return 1;
    default: return !set_is_infinite(s);
    }
}

NATIVE(n_isfinite) { UNUSED; return BOOLV(tlc_finite(a[0])); }

NATIVE(n_cardinality) {
    UNUSED;
    if (!is_set(a[0]) || user_set(a[0])) fail_module("Attempted to compute cardinality of the value\n%s", pretty(a[0]));
    if (set_is_infinite(a[0])) count_infinite(a[0]);
    int64_t n = set_size(union_cache(a[0]));
    if (n < 0)
        fail("Overflow when computing the number of elements in%s%s",
                    a[0]->lazy == L_TIMES ? " " : ":\n", pretty(a[0]));
    return mk_int(n);
}

/* v, as TLC prints it: it enumerates an infinite S \cup T or S \ T
   inside, which fails. */
static Value *printable(Value *v) {
    if (v->kind == V_LAZY && v->lazy >= L_UNION) enumerate(v);
    if (v->kind == V_SET) for (int i = 0; i < v->n; i++) printable(v->u.set.elems[i]);
    if (is_fcn(v)) for (int i = 0; i < v->n; i++) printable(v->u.fcn.vals[i]);
    return v;
}

static FILE *user_file;   /* where Print writes, -userFile; NULL for standard output */
static int reprinting;    /* a constant computed again, which has printed */

NATIVE(n_print) {
    UNUSED;
    printable(a[0]), printable(a[1]);
    if (in_worker) worker_cannot();   /* the search prints in order */
    FILE *out = user_file ? user_file : stdout;
    if (!reprinting) fprintf(out, "%s  %s\n", pretty(a[0]), pretty(a[1]));
    fflush(out);
    return a[1];
}

NATIVE(n_printt) {
    UNUSED;
    if (in_worker) worker_cannot();
    FILE *out = user_file ? user_file : stdout;
    if (!reprinting) fprintf(out, "%s\n", pretty(printable(a[0])));
    fflush(out);
    return TRUEV;
}

NATIVE(n_assert) {
    UNUSED;
    if (a[0]->kind == V_BOOL && a[0]->u.i) return TRUEV;
    error_reset();
    buf_printf(&error_message, "The first argument of Assert evaluated to FALSE; the second argument was:\n%s", pretty(a[1]));
    error_status = TPP_VIOLATION_ASSERT;
    error_module = 1;
    raise_error();
}

NATIVE(n_javatime) { UNUSED; return mk_int((int64_t)time(NULL) & 0x7fffffff); }

static void encode(Buf *b, Value *v);
static Value *decode(const char **p);

/* TLCSet's registers, each its value encoded, as values do not outlive
   the state being expanded. */
static Buf *registers;
static int nregisters;
static int starting;   /* the model is being read: TLC's checker, which TLCGet asks, does not exist yet */
static int64_t search_counter(const char *key);

NATIVE(n_tlcget) {
    UNUSED;
    if (starting) fail_module("TLCGet is evaluated before TLC starts checking.");
    if (a[0]->kind == V_STR) {
        const char *key = S(a[0]->u.i);
        if (!strcmp(key, "level")) return mk_int(cur_level);
        int64_t n = search_counter(key);
        if (n < 0) fail_module("TLCGet(%s) was undefined.", key);
        return mk_int(n);
    }
    if (a[0]->kind != V_INT || a[0]->u.i < 0)
        fail_module("The argument of TLCGet should be a nonnegative integer, but instead it is:\n%s", pretty(a[0]));
    if (a[0]->u.i >= nregisters || !registers[a[0]->u.i].n) fail_module("TLCGet(%s) was undefined.", pretty(a[0]));
    const char *p = registers[a[0]->u.i].s;
    return decode(&p);
}

NATIVE(n_tlcset) {
    UNUSED;
    if (a[0]->kind == V_STR && !strcmp(S(a[0]->u.i), "exit")) {   /* the search stops after this state */
        if (a[1]->kind == V_BOOL && a[1]->u.i) {
            if (in_worker) worker_cannot();
            exit_requested = 1;
        }
        return TRUEV;
    }
    if (a[0]->kind != V_INT || a[0]->u.i < 0)
        fail_module("The first argument of TLCSet should be a nonnegative integer, but instead it is:\n%s", pretty(a[0]));
    int i = (int)a[0]->u.i;
    if (i >= nregisters) {
        registers = xrealloc(registers, sizeof(Buf) * (size_t)(i + 1));
        memset(registers + nregisters, 0, sizeof(Buf) * (size_t)(i + 1 - nregisters));
        nregisters = i + 1;
    }
    registers[i].n = 0;
    encode(&registers[i], a[1]);
    return TRUEV;
}

NATIVE(n_singleton) {
    UNUSED;
    Value *f = mk_fcn(1);
    f->u.fcn.keys[0] = a[0];
    f->u.fcn.vals[0] = a[1];
    return f;
}

NATIVE(n_merge) {
    UNUSED;
    for (int i = 0; i < 2; i++) {
        a[i] = realize(a[i]);
        if (a[i]->kind == V_LAMBDA) enumerate(a[i]->u.l.a);   /* TLC enumerates the domain, which fails */
        if (!is_fcn(a[i]))
            fail_module("The %s argument of @@ should be a function, but instead it is:\n%s", i ? "second" : "first", pretty(a[i]));
    }
    List keys = {0}, vals = {0};
    for (int i = 0; i < a[0]->n; i++) { list_add(&keys, fcn_key(a[0], i)); list_add(&vals, a[0]->u.fcn.vals[i]); }
    for (int i = 0; i < a[1]->n; i++) {
        Value *k = fcn_key(a[1], i);
        int seen = 0;
        for (int j = 0; j < a[0]->n && !seen; j++) seen = veq(k, fcn_key(a[0], j));
        if (!seen) { list_add(&keys, k); list_add(&vals, a[1]->u.fcn.vals[i]); }
    }
    return mk_fcn_from(&keys, &vals);
}

NATIVE(n_permutations) {
    UNUSED;
    Value *s = enumerate(a[0]);
    int n = s->n;
    if (n > 10) fail_module("Attempted to compute Permutations of a set with more than 10 elements.");
    List out = {0};
    int *p = NEWN(int, n);
    for (int i = 0; i < n; i++) p[i] = i;
    for (;;) {
        Value *f = mk_fcn(n);
        for (int i = 0; i < n; i++) { f->u.fcn.keys[i] = s->u.set.elems[i]; f->u.fcn.vals[i] = s->u.set.elems[p[i]]; }
        list_add(&out, f);
        int i = n - 2;
        while (i >= 0 && p[i] >= p[i + 1]) i--;
        if (i < 0) break;
        int j = n - 1;
        while (p[j] <= p[i]) j--;
        int t = p[i]; p[i] = p[j]; p[j] = t;
        for (int l = i + 1, r = n - 1; l < r; l++, r--) { t = p[l]; p[l] = p[r]; p[r] = t; }
    }
    return list_set(&out);
}

NATIVE(n_sortseq) {
    UNUSED;
    Value *x = seq_of(seq_arg(a[0], NOT_SEQ("first argument", "SortSeq")));
    Value *t = mk_tuple(x->n);
    for (int i = 0; i < x->n; i++) t->u.fcn.vals[i] = x->u.fcn.vals[i];
    for (int i = 1; i < t->n; i++) {
        Value *v = t->u.fcn.vals[i];
        int j = i - 1;
        while (j >= 0) {
            Value *pair[2] = {v, t->u.fcn.vals[j]};
            Value *r = apply_oper(a[1], pair, 2, call);
            if (r->kind != V_BOOL) fail_module("The second argument of SortSeq should be a function, but instead it is:\n%s", pretty(r));
            if (!r->u.i) break;
            t->u.fcn.vals[j + 1] = t->u.fcn.vals[j];
            j--;
        }
        t->u.fcn.vals[j + 1] = v;
    }
    return t;
}

/* java.util.Random, the generator TLC draws from. */
typedef struct { uint64_t s; } JRandom;
#define JRANDOM(seed) {((uint64_t)(seed) ^ 0x5DEECE66DULL) & ((1ULL << 48) - 1)}

static void jr_seed(JRandom *r, int64_t seed) { *r = (JRandom)JRANDOM(seed); }

static int32_t jr_next(JRandom *r, int bits) {
    r->s = (r->s * 0x5DEECE66DULL + 0xB) & ((1ULL << 48) - 1);
    return (int32_t)(uint32_t)(r->s >> (48 - bits));
}

static int64_t jr_long(JRandom *r) {
    uint64_t hi = (uint64_t)(int64_t)jr_next(r, 32);
    return (int64_t)((hi << 32) + (uint64_t)(int64_t)jr_next(r, 32));
}

static double jr_double(JRandom *r) { return (double)(((int64_t)jr_next(r, 26) << 27) + jr_next(r, 27)) * 0x1.0p-53; }

/* Below bound, as nextInt(bound). */
static int jr_int(JRandom *r, int bound) {
    int32_t x = jr_next(r, 31), m = bound - 1;
    if (!(bound & m)) return (int)(((int64_t)bound * x) >> 31);
    for (int32_t u = x; (int32_t)((uint32_t)u - (uint32_t)(x = u % bound) + (uint32_t)m) < 0; u = jr_next(r, 31)) {}
    return x;
}

/* What RandomElement and the Randomization module draw from: TLC's
   RandomEnumerableValues. A simulation draws as TLC 2.19's threads do,
   from java.util.Random(0): one for the model and the initial states,
   another for the behaviors. Model checking draws from java.util.Random
   seeded with -seed, again for Init and for each state expanded, xored
   with the state's fingerprint so that a trace can be rebuilt; tpp's
   fingerprints are not TLC's, so there the draws are not TLC's. */
static JRandom draw_model = JRANDOM(0), draw_behaviors = JRANDOM(0), *drawing = &draw_model;
static int64_t draw_seed;
static Value **draw_from;   /* the state expanded, NULL for Init */
static int draw_due;        /* to seed at the next draw */
static uint64_t fingerprint(Value **st);

static void draw_restart(Value **from) { draw_from = from; draw_due = 1; }

static JRandom *draws(void) {
    if (draw_due) jr_seed(drawing, draw_seed ^ (int64_t)(draw_from ? fingerprint(draw_from) : 0));
    draw_due = 0;
    return drawing;
}

/* A set of functions, records or tuples: TLC draws an element of each
   part's set. Its parts' sets, in its element's order, for n_parts. */
static int is_product(Value *s) { return s->kind == V_LAZY && (s->lazy == L_FCNSET || s->lazy == L_RCDSET || s->lazy == L_TIMES); }

static Value **product_parts(Value *s, int *n) {
    if (s->lazy != L_FCNSET) { *n = s->n; return s->u.l.names + s->n; }
    *n = enumerate(s->u.l.a)->n;
    Value **sets = NEWN(Value *, *n);
    for (int i = 0; i < *n; i++) sets[i] = s->u.l.b;
    return sets;
}

/* The element of product s whose parts are vals. */
static Value *product_element(Value *s, Value **vals) {
    Value *dom = s->lazy == L_FCNSET ? enumerate(s->u.l.a) : NULL;
    int n = dom ? dom->n : s->n;
    Value *t = s->lazy == L_TIMES ? mk_tuple(n) : mk_fcn(n);
    for (int i = 0; i < n; i++) {
        if (s->lazy != L_TIMES) t->u.fcn.keys[i] = dom ? dom->u.set.elems[i] : s->u.l.names[i];
        t->u.fcn.vals[i] = vals[i];
    }
    if (s->lazy == L_RCDSET) t->made = MADE_RECORD;
    return t;
}

static Value *random_element(Value *s) {
    if (s->kind == V_SET && s->u.set.product) s = s->u.set.product;
    if (is_product(s) && !(s->lazy == L_FCNSET && set_is_infinite(s->u.l.a))) {
        int n;
        Value **sets = product_parts(s, &n), **vals = NEWN(Value *, n);
        for (int i = 0; i < n; i++) vals[i] = random_element(sets[i]);
        return product_element(s, vals);
    }
    Value *e = enumerate(s);
    if (!e->n) fail_module("Attempted to compute RandomElement of the empty set.");
    return e->u.set.elems[(int)(jr_double(draws()) * e->n)];
}

NATIVE(n_randomelement) { UNUSED; return random_element(a[0]); }

/* k elements of S, as TLC 2.19 draws them: k indices into S apart by a
   fixed step from a random one. A set of functions or records too large
   for an int index has 63-bit steps from a random long, and TLC decodes
   a function's index by powers of doubles. */
NATIVE(n_randomsubset) {
    UNUSED;
    if (a[0]->kind != V_INT)
        fail_module("The first argument of RandomSubset should be a nonnegative integer, but instead it is:\n%s", pretty(a[0]));
    if (!is_set(a[1]) || set_is_infinite(a[1]))
        fail_module("The second argument of RandomSubset should be a a finite set, but instead it is:\n%s", pretty(a[1]));
    Value *s = a[1], **sets = NULL, *e = NULL;
    int n = 0;
    __extension__ typedef __int128 Big;
    const Big huge = (Big)1 << 126;   /* above every index drawn */
    Big size = 1;
    if (is_product(s) && s->lazy != L_TIMES) {
        sets = product_parts(s, &n);
        Value **parts = NEWN(Value *, n);
        for (int i = 0; i < n; i++) {
            parts[i] = enumerate(sets[i]);
            size = size > huge / (parts[i]->n + 1) ? huge : size * parts[i]->n;
        }
        sets = parts;
    } else {
        e = enumerate(s);
        size = e->n;
    }
    JRandom *r = draws();
    int big = size > 2147483647, k = (int)a[0]->u.i;
    Big x = big ? INT64_MAX - 24 : size < 191 ? 191 : 2147483647, start = 0;
    if (big) {
        int64_t l = jr_long(r);
        start = l < 0 && l != INT64_MIN ? -l : l;
    } else if (size > 0) start = jr_int(r, (int)size);
    else k = 0;
    List l = {0};
    Value **vals = NEWN(Value *, n);
    for (int i = 0; i < k; i++) {
        Big idx = (x * i + start) % size;
        if (idx < 0) idx += size;
        if (e) { list_add(&l, e->u.set.elems[(int64_t)idx]); continue; }
        Big scale = 1;   /* of the last part, the least significant */
        for (int j = n - 1, p = 0; j >= 0; j--, p++) {
            int mod = sets[j]->n;
            if (big && s->lazy == L_FCNSET) {
                double d = pow(mod, p);
                scale = d >= 0x1p63 ? INT64_MAX : (int64_t)d;
            }
            vals[j] = sets[j]->u.set.elems[(int64_t)(idx / scale % mod)];
            if (!big || s->lazy != L_FCNSET) scale = scale > huge / (mod + 1) ? huge : scale * mod;
        }
        list_add(&l, product_element(s, vals));
    }
    return list_set(&l);
}

/* k subsets of S, as TLC draws them: each element in with probability
   n / Cardinality(S). */
NATIVE(n_randomsetofsubsets) {
    UNUSED;
    for (int i = 0; i < 2; i++)
        if (a[i]->kind != V_INT || a[i]->u.i < 0)
            fail_module("The %s argument of RandomSetOfSubsets should be a nonnegative integer, but instead it is:\n%s",
                        i ? "second" : "first", pretty(a[i]));
    if (!is_set(a[2]) || set_is_infinite(a[2]))
        fail_module("The third argument of RandomSetOfSubsets should be a finite set, but instead it is:\n%s", pretty(a[2]));
    Value *s = enumerate(a[2]);
    int64_t k = a[0]->u.i, n = a[1]->u.i;
    if (s->n < 63 && k > ((int64_t)1 << s->n))
        fail_module("The first argument of RandomSetOfSubsets should be a nonnegative integer that is smaller than "
                    "the subset's size of 2^%d, but instead it is:\n%lld", s->n, (long long)k);
    if (n > s->n)
        fail_module("The second argument of RandomSetOfSubsets should be a nonnegative integer in range "
                    "0..Cardinality(S), but instead it is:\n%s", pretty(a[1]));
    double p = (double)n / s->n;
    JRandom *r = draws();
    List l = {0};
    for (int64_t j = 0; j < k; j++) {
        List sub = {0};
        for (int i = 0; i < s->n; i++) if (jr_double(r) < p) list_add(&sub, s->u.set.elems[i]);
        list_add(&l, list_set(&sub));
    }
    return list_set(&l);
}

NATIVE(n_any) { UNUSED; return mk_lazy(L_ANY); }

NATIVE(n_tostring) {
    UNUSED;
    char *s = value_str(printable(a[0]));
    return mk_str(intern(s));
}

NATIVE(n_tlceval) { UNUSED; return a[0]; }

/* Java's names of TLC's value classes, for its methods' signatures. */
#define J_V "tlc2.value.impl.Value"
#define J_I "tlc2.value.impl.IntValue"
#define J_IV "tlc2.value.impl.IntervalValue"
#define J_B "tlc2.value.IBoolValue"

/* Each native operator, with the Java method TLC implements it by. */
static const struct { const char *module, *name; Native fn; const char *ret, *method; } natives[] = {
    {"Naturals", "Nat", n_nat, J_V, "Nat()"},
    {"Naturals", "+", n_plus, J_I, "Plus(" J_I "," J_I ")"},
    {"Naturals", "-", n_minus, J_I, "Minus(" J_I "," J_I ")"},
    {"Naturals", "*", n_times, J_I, "Times(" J_I "," J_I ")"},
    {"Naturals", "^", n_pow, J_I, "Expt(" J_I "," J_I ")"},
    {"Naturals", "<", n_lt, J_B, "LT(" J_V "," J_V ")"},
    {"Naturals", ">", n_gt, J_B, "GT(" J_V "," J_V ")"},
    {"Naturals", "\\leq", n_leq, J_B, "LE(" J_V "," J_V ")"},
    {"Naturals", "\\geq", n_geq, J_B, "GEQ(" J_V "," J_V ")"},
    {"Naturals", "%", n_mod, J_I, "Mod(" J_I "," J_I ")"},
    {"Naturals", "\\div", n_div, J_I, "Divide(" J_I "," J_I ")"},
    {"Naturals", "..", n_range, J_IV, "DotDot(" J_I "," J_I ")"},
    {"Integers", "Int", n_int, J_V, "Int()"},
    {"Integers", "-.", n_neg, J_I, "Neg(" J_I ")"},
    {"Sequences", "Seq", n_seq, J_V, "Seq(" J_V ")"},
    {"Sequences", "Len", n_len, J_I, "Len(" J_V ")"},
    {"Sequences", "\\o", n_concat, J_V, "Concat(" J_V "," J_V ")"},
    {"Sequences", "Append", n_append, J_V, "Append(" J_V "," J_V ")"},
    {"Sequences", "Head", n_head, J_V, "Head(" J_V ")"},
    {"Sequences", "Tail", n_tail, J_V, "Tail(" J_V ")"},
    {"Sequences", "SubSeq", n_subseq, J_V, "SubSeq(" J_V "," J_V "," J_V ")"},
    {"Sequences", "SelectSeq", n_selectseq, J_V, "SelectSeq(" J_V "," J_V ")"},
    {"FiniteSets", "IsFiniteSet", n_isfinite, J_B, "IsFiniteSet(" J_V ")"},
    {"FiniteSets", "Cardinality", n_cardinality, J_I, "Cardinality(" J_V ")"},
    {"TLC", "Print", n_print, J_V, "Print(" J_V "," J_V ")"},
    {"TLC", "PrintT", n_printt, J_V, "PrintT(" J_V ")"},
    {"TLC", "Assert", n_assert, J_V, "Assert(" J_V "," J_V ")"},
    {"TLC", "JavaTime", n_javatime, J_V, "JavaTime()"},
    {"TLC", "TLCGet", n_tlcget, J_V, "TLCGet(" J_V ")"},
    {"TLC", "TLCSet", n_tlcset, J_V, "TLCSet(" J_V "," J_V ")"},
    {"TLC", ":>", n_singleton, J_V, "MakeFcn(" J_V "," J_V ")"},
    {"TLC", "@@", n_merge, J_V, "CombineFcn(" J_V "," J_V ")"},
    {"TLC", "Permutations", n_permutations, J_V, "Permutations(" J_V ")"},
    {"TLC", "SortSeq", n_sortseq, J_V, "SortSeq(" J_V "," J_V ")"},
    {"TLC", "RandomElement", n_randomelement, J_V, "RandomElement(" J_V ")"},
    {"TLC", "Any", n_any, J_V, "Any()"},
    {"Randomization", "RandomSubset", n_randomsubset, J_V, "RandomSubset(" J_V "," J_V ")"},
    {"Randomization", "RandomSetOfSubsets", n_randomsetofsubsets, J_V, "RandomSetOfSubsets(" J_V "," J_V "," J_V ")"},
    {"TLC", "ToString", n_tostring, J_V, "ToString(" J_V ")"},
    {"TLC", "TLCEval", n_tlceval, J_V, "TLCEval(" J_V ")"},
    {NULL, NULL, NULL, NULL, NULL}};

/* The number of arguments of a native's Java method. */
static int native_arity(const char *method) {
    const char *p = strchr(method, '(');
    int n = p[1] != ')';
    for (; *p; p++) n += *p == ',';
    return n;
}

/* Attaches the native operators to the modules of the standard modules'
   names, as TLC loads its classes by the module's name: one the user
   supplies too, where an operator of the native's arity is the native. */
static void bind_natives(void) {
    for (int i = 0; i < nmodules; i++) {
        Module *m = modules[i];
        for (int u = 0; u < m->nunits; u++) {
            Def *d = m->units[u].def;
            if (m->units[u].kind != 0 || !d) continue;
            for (int k = 0; natives[k].module; k++)
                if (strcmp(natives[k].module, S(m->name)) == 0 && strcmp(natives[k].name, S(d->name)) == 0
                    && d->nparams == native_arity(natives[k].method)) {
                    d->native = natives[k].fn;
                    /* TLC always loads Integers (its trace modules use it), and takes
                       Naturals' operators from its class. */
                    const char *cls = strcmp(natives[k].module, "Naturals") == 0 ? "Integers" : natives[k].module;
                    Buf b = {0};
                    buf_printf(&b, "public static %s tlc2.module.%s.%s", natives[k].ret, cls, natives[k].method);
                    d->java = buf_take(&b);
                }
        }
    }
}

/* ---------------------------------------------------------------- generator */

/* The generator enumerates the assignments an initial predicate or an
   action allows (§10.2, §10.3). Conjuncts still to be satisfied wait in a
   continuation. */
typedef struct Cont { Expr *e; Ctx c; struct Cont *next; int unchanged; } Cont;   /* unchanged: the conjunct is UNCHANGED e */

static Value **gen_target;      /* the state being assigned */
static int (*gen_emit)(void);   /* called with each complete assignment; nonzero stops */

static int gen(Expr *e, Ctx c, Cont *k);
static int gen_unchanged(Expr *e, Ctx c, Cont *k);

static _Noreturn void not_boolean(Expr *e, Value *v);

static int gen_cont(Cont *k) {
    if (!k) return gen_emit();
    int assigned = 1;
    for (int i = 0; i < nvars && assigned; i++) assigned = gen_target[i] != NULL;
    if (!assigned) return k->unchanged ? gen_unchanged(k->e, k->c, k->next) : gen(k->e, k->c, k->next);
    /* Once every variable has a value, TLC evaluates the remaining
       conjuncts as formulas: a disjunction stops at its first true
       disjunct. */
    for (; k; k = k->next) {
        Value *v = k->unchanged ? BOOLV(kept(k->e, k->c)) : eval(k->e, k->c);
        if (v->kind != V_BOOL) not_boolean(k->e, v);
        if (!v->u.i) return 0;
    }
    return gen_emit();
}

static Cont *mk_cont(Expr *e, Ctx c, Cont *next) {
    Cont *k = NEW(Cont);
    k->e = e; k->c = c; k->next = next;
    return k;
}

/* A variable reached from where ENABLED marks the chain, not through an
   argument from outside, is bound by name, in a slot past the state's own
   when no state variable has that name. */
static int enabled_names[16], nenabled_names;

static int enabled_slot(Def *d) {
    for (int i = 0; i < nvars; i++) if (vars[i]->name == d->name) return i;
    for (int i = 0; i < nenabled_names; i++) if (enabled_names[i] == d->name) return nvars + i;
    if (nenabled_names == 16) return -1;
    enabled_names[nenabled_names] = d->name;
    return nvars + nenabled_names++;
}

/* The evaluation globals, which a caller that goes on after an error
   restores. */
typedef struct {
    sigjmp_buf *handler;
    Arena *arena;
    Value **cur_state, **next_state, **gen_target, *at_value;
    int (*gen_emit)(void);
    int primed, gen_init, nenabled_names, java_depth, eval_depth, keep_lazy, fp_depth, nregions;
} EvalState;

static EvalState eval_save(void) {
    return (EvalState){handler, arena, cur_state, next_state, gen_target, at_value, gen_emit,
                       primed, gen_init, nenabled_names, java_depth, eval_depth, keep_lazy, fp_depth, nregions};
}

static void eval_restore(EvalState s) {
    handler = s.handler; arena = s.arena;
    cur_state = s.cur_state; next_state = s.next_state; gen_target = s.gen_target; at_value = s.at_value;
    gen_emit = s.gen_emit;
    primed = s.primed; gen_init = s.gen_init; nenabled_names = s.nenabled_names;
    java_depth = s.java_depth; eval_depth = s.eval_depth; keep_lazy = s.keep_lazy; fp_depth = s.fp_depth;
    nregions = s.nregions;
}

/* The error being reported, which an evaluation whose own error is
   forgotten keeps. */
typedef struct { Buf msg; Expr *expr; int status, module, stackless; } ErrorState;

static ErrorState error_save(void) {
    ErrorState e = {{0}, error_expr, error_status, error_module, stackless};
    buf_puts(&e.msg, error_message.s ? error_message.s : "");
    return e;
}

static void error_restore(ErrorState *e) {
    error_reset();
    buf_puts(&error_message, e->msg.s);
    free(e->msg.s);
    error_expr = e->expr, error_status = e->status, error_module = e->module, stackless = e->stackless;
}

/* A lazy function as TLC prints it: computed in full if it can be, else
   as its definition, the error forgotten (§6.6). A worker gives the state
   back instead, as the error may be one only the search can avoid. */
static Value *try_realize(Value *f) {
    if (in_worker || set_is_infinite(f->u.l.a)) return realize(f);
    EvalState s = eval_save();
    ErrorState e = error_save();
    Value *volatile r = f;
    sigjmp_buf here;
    handler = &here;
    if (!sigsetjmp(here, 0)) r = realize(r);
    eval_restore(s);
    error_restore(&e);
    return r;
}

static void print_lambda(Buf *b, Value *f) {
    Bound *bs = f->u.l.binder ? f->u.l.binder->bounds : f->u.l.expr->bounds;
    int nb = f->u.l.binder ? f->u.l.binder->nb : f->u.l.expr->nb;
    int total = bounds_total(bs, nb), k = 0;
    buf_puts(b, "[");
    for (int i = 0; i < nb; i++) {
        if (i) buf_puts(b, ", ");
        if (bs[i].tuple) {
            buf_puts(b, "<<");
            for (int j = 0; j < bs[i].n; j++) buf_printf(b, "%s%s", j ? ", " : "", S(bs[i].names[j]));
            buf_puts(b, ">>");
        } else {
            buf_puts(b, S(bs[i].names[0]));
        }
        buf_puts(b, " \\in ");
        print_value(b, total == 1 ? f->u.l.a : f->u.l.a->u.l.names[total + k]);
        k += bs[i].tuple ? 1 : bs[i].n;
    }
    buf_puts(b, " |-> <expression ");
    node_text(b, f->u.l.binder ? f->u.l.binder->body : f->u.l.expr->args[0]);
    buf_puts(b, ">]");
}

/* A filter of a set TLC cannot enumerate, as it prints one: a tuple of
   names without its brackets. */
static void print_filter(Buf *b, Value *s) {
    Bound *bound = &s->u.l.expr->bounds[0];
    buf_puts(b, "{");
    for (int j = 0; j < bound->n; j++) buf_printf(b, "%s%s", j ? ", " : "", S(bound->names[j]));
    buf_puts(b, " \\in ");
    print_value(b, s->u.l.a);
    buf_puts(b, " : <expression ");
    node_text(b, s->u.l.expr->args[0]);
    buf_puts(b, "> }");
}

/* Binds the bounds bs over dom (as bounds_domain makes it) to the
   argument a, with TLC's checks: 0, or 1 if a part is outside its set and
   2 if it is not a tuple where one is needed, that part being *argno (0:
   the only one, -1: the whole argument). */
static int bind_arg(Bound *bs, int nb, Value *dom, Value *a, Env **env, int *argno) {
    int total = bounds_total(bs, nb), k = 0;
    *argno = total == 1 ? 0 : -1;
    Value **parts = &a;
    if (total > 1) {
        if (!is_fcn(a) || !fcn_is_tuple(a) || a->n < total) return 2;
        parts = a->u.fcn.vals;
    }
    for (int i = 0; i < nb; i++) {
        Bound *b = &bs[i];
        for (int j = 0; j < (b->tuple ? 1 : b->n); j++, k++) {
            Value *part = parts[k], *set = total == 1 ? dom : dom->u.l.names[total + k];
            if (total > 1) *argno = k + 1;
            if (!set_member(part, set)) return b->tuple && total > 1 ? 3 : 1;
            if (!b->tuple) { *env = mk_binding(b->defs[j], part, *env); continue; }
            if (!is_fcn(part) || !fcn_is_tuple(part) || part->n != b->n) return 2;
            for (int m = 0; m < b->n; m++) *env = mk_binding(b->defs[m], part->u.fcn.vals[m], *env);
        }
    }
    return 0;
}

static _Noreturn void lambda_outside(Value *f, Value *a, int why, int argno, Expr *where) {
    char part[48];
    if (argno == 0) snprintf(part, sizeof part, "the first argument");
    else if (argno < 0) snprintf(part, sizeof part, "the argument list");
    else snprintf(part, sizeof part, "the argument number %d", argno);
    if (argno > 0) a = a->u.fcn.vals[argno - 1];
    const char *wrong = why == 2 ? "which does not match its formal parameter." : "which is not in its domain.";
    if (!where && why == 1 && argno > 0) {   /* a part of several outside its set: TLC names the whole domain */
        const char *dom = pretty(enumerate(f->u.l.a));
        fail_module("In applying the function\n%s,\n%s is:\n%s\nwhich is not in the function's domain %s.\n", pretty(f), part, pretty(a), dom);
    }
    if (!where) fail_module("In applying the function\n%s,\n%s is:\n%s\n%s", pretty(f), part, pretty(a), wrong);
    /* Computing states, TLC binds the argument itself (Tool.getFcnContext). */
    Buf b = {0};
    loc_text(&b, where);
    if (argno < 0) fail("Attempted to apply a function to an argument not in its domain.\n%s", b.s);
    fail("In applying the function\n%s,\n%s is:\n%s%s%s\n%s", pretty(f), argno || why != 2 ? part : "the argument",
         pretty(a), why == 3 ? "\n" : "", wrong, b.s);
}

/* The variable an expression assigns: v (initial predicate) or v'
   (action), through operator parameters, definitions and instance
   substitutions that stand for it; -1 if none. */
static int assignable(Expr *lhs, Ctx c) {
    if (!gen_init) {
        /* A parameter may stand for a primed argument: Op(x') == x = 1. */
        for (Env *b; lhs->kind == K_BOUND && (b = env_find(c.env, lhs->def)) && b->e; c = (Ctx){b->ctx_env, b->ctx_inst})
            lhs = b->e;
        if (lhs->kind != K_PRIME) return -1;
        lhs = lhs->args[0];
    }
    /* Through parameters, definitions and substitutions, as TLC's getVar. */
    for (;;) {
        Def *d = lhs->def;
        if (lhs->kind == K_BOUND || (lhs->kind == K_CALL && lhs->n == 0)) {
            if (lhs->kind == K_BOUND || d->kind == D_PARAM || d->local == 2) {
                Env *b = env_find(c.env, d);
                if (!b || !b->e) return -1;
                lhs = b->e;
                c = (Ctx){b->ctx_env, b->ctx_inst};
            } else if (d->kind == D_OP && !d->native && !d->nparams && !d->replacement) {
                lhs = d->body;
                c = (Ctx){NULL, c.inst};
            } else {
                return -1;
            }
        } else if (lhs->kind == K_DECL) {
            Expr *sub = NULL;
            Ctx outer = c;
            for (Inst *in = c.inst; in && !sub; in = in->parent) {
                if (in->def == &enabled_mark) return d->kind == D_VAR ? enabled_slot(d) : -1;
                for (int i = 0; i < in->def->nsubst; i++)
                    if (in->def->subst_decl[i] == d) { sub = in->def->subst_expr[i]; outer = (Ctx){in->env, in->parent}; break; }
            }
            if (!sub) return d->kind == D_VAR ? d->var_index : -1;
            lhs = sub;
            c = outer;
        } else {
            return -1;
        }
    }
}

static int gen_assign(int var, Value *v, Cont *k) {
    gen_target[var] = v;
    int r = gen_cont(k);
    gen_target[var] = NULL;
    return r;
}

typedef struct { Expr *body; Cont *k; } GenData;

static int gen_bound_fn(Ctx c, void *data) {
    GenData *g = data;
    return gen(g->body, c, g->k);
}

/* The bindings of a quantifier, in the arena. */
typedef struct { Ctx *v; int n, cap; } Ctxs;

static int collect_fn(Ctx c, void *data) {
    Ctxs *l = data;
    if (l->n == l->cap) {
        Ctx *v = NEWN(Ctx, l->cap = l->cap ? 2 * l->cap : 8);
        if (l->n) memcpy(v, l->v, sizeof(Ctx) * (size_t)l->n);
        l->v = v;
    }
    l->v[l->n++] = c;
    return 0;
}

/* UNCHANGED e: each variable of e keeps its value. */
/* Whether e' = e, comparing each variable with the value it has been
   assigned: in ENABLED, a variable of an instance by name, as TLC does. */
static int unchanged_now(Expr *e, Ctx c) {
    if (e->kind == X_TUPLE) {
        for (int i = 0; i < e->n; i++) if (!unchanged_now(e->args[i], c)) return 0;
        return 1;
    }
    if (e->kind == K_CALL && e->n == 0 && e->def->kind == D_OP && !e->def->native && e->def->local != 2)
        return unchanged_now(e->def->body, (Ctx){NULL, c.inst});
    if (e->kind == K_DECL) {
        Expr prime = {.kind = K_PRIME, .n = 1, .args = &e};
        int var = assignable(&prime, c);
        if (var >= 0 && gen_target[var]) return veq(eval(e, c), gen_target[var]);
    }
    Value *now = eval(e, c);
    int saved = primed;
    primed = 1;
    Value *then = eval(e, c);
    primed = saved;
    return veq(now, then);
}

static int gen_unchanged(Expr *e, Ctx c, Cont *k) {
    if (e->kind == X_TUPLE) {
        for (int i = e->n - 1; i >= 1; i--) {
            k = mk_cont(e->args[i], c, k);
            k->unchanged = 1;
        }
        if (e->n == 0) return gen_cont(k);
        return gen_unchanged(e->args[0], c, k);
    }
    if (e->kind == K_CALL && e->n == 0 && e->def->kind == D_OP && !e->def->native && e->def->local != 2)
        return gen_unchanged(e->def->body, (Ctx){NULL, c.inst}, k);
    if (e->kind == K_BOUND) {   /* a parameter: of its argument */
        Env *b = env_find(c.env, e->def);
        if (b && b->e) return gen_unchanged(b->e, (Ctx){b->ctx_env, b->ctx_inst}, k);
    }
    if (e->kind == K_DECL) {
        /* UNCHANGED of a variable an instance substitutes: of what it is. */
        Ctx outer = c;
        Expr *sub = substitute(e->def, &outer);
        int named = 0;   /* ENABLED binds the variable by name */
        for (Inst *in = c.inst; in != outer.inst && !named; in = in->parent) named = in->def == &enabled_mark;
        if (sub && sub->kind != X_OPREF && !named) return gen_unchanged(sub, outer, k);
        Expr prime;
        memset(&prime, 0, sizeof prime);
        prime.kind = K_PRIME;
        prime.n = 1;
        prime.args = &e;
        int var = assignable(&prime, c);
        if (var >= 0 && !gen_target[var]) {
            Value *v = eval(e, c);
            return gen_assign(var, v, k);
        }
    }
    return unchanged_now(e, c) ? gen_cont(k) : 0;
}

static _Noreturn void not_boolean(Expr *e, Value *v) {
    (void)v;
    fail_at(e, "TLC expected a boolean value, but did not find one.");
}

/* The context of the body of an instance call's definition: the chain
   of instances (instance_chain) and the definition's parameters. */
static Ctx instance_ctx(Expr *e, Ctx c) {
    Def *d = e->def;
    Inst *inst = instance_chain(e, c);
    Env *env = NULL;
    for (int i = 0; i < e->n && i < d->nparams; i++) {
        if (d->params[i]->arity) env = mk_binding(d->params[i], eval_operator_arg(e->args[i], c), env);
        else env = mk_arg(d->params[i], e->args[i], c, env);
    }
    return (Ctx){env, inst};
}

static int gen(Expr *e, Ctx c, Cont *k) {
    switch (e->kind) {
    case X_BOOL:
        return e->num ? gen_cont(k) : 0;
    case K_AND:
        for (int i = e->n - 1; i >= 1; i--) k = mk_cont(e->args[i], c, k);
        return gen(e->args[0], c, k);
    case K_OR:
        for (int i = 0; i < e->n; i++) if (gen(e->args[i], c, k)) return 1;
        return 0;
    case X_EXISTS: {
        GenData g = {e->args[0], k};
        return for_bindings(e->bounds, e->nb, c, gen_bound_fn, &g);
    }
    case X_FORALL: {
        /* As TLC: the conjunction of the body for each binding, the
           first, then the others from the last, each a choice point. */
        Ctxs l = {0};
        for_bindings(e->bounds, e->nb, c, collect_fn, &l);
        if (!l.n) return gen_cont(k);
        for (int i = 1; i < l.n; i++) k = mk_cont(e->args[0], l.v[i], k);
        return gen(e->args[0], l.v[0], k);
    }
    case X_IF: {
        Value *v = eval(e->args[0], c);
        if (v->kind != V_BOOL) not_boolean(e->args[0], v);
        return gen(e->args[v->u.i ? 1 : 2], c, k);
    }
    case K_IMPLIES: {
        Value *v = eval(e->args[0], c);
        if (v->kind != V_BOOL && gen_init) {
            Buf b = {0};
            loc_text(&b, e);
            fail("In computing initial states of a predicate of form P => Q, P was %s\n.%s", article_kind(v), b.s);
        }
        if (v->kind != V_BOOL) fail_at(e, "In computing next states of a predicate of the form P => Q, P was\n%s.", article_kind(v));
        return v->u.i ? gen(e->args[1], c, k) : gen_cont(k);
    }
    case X_CASE:
        for (int i = 0; i + 1 < e->n; i += 2)
            if (!e->args[i] || eval_bool(e->args[i], c)) return gen(e->args[i + 1], c, k);
        fail_at(e, "Attempted to evaluate a CASE with no conditions true.");
    case X_LET: {
        Env *env = c.env;
        for (int i = 0; i < e->ndefs; i++) env = mk_thunk(e->defs[i], e->defs[i]->body, c, env);
        for (Env *b = env; b != c.env; b = b->next) b->ctx_env = env;
        return gen(e->args[0], (Ctx){env, c.inst}, k);
    }
    case K_CALL: {
        Def *d = e->def->replacement ? e->def->replacement : e->def;
        if (d->kind != D_OP || d->native || d->nparams != e->n) break;
        if (d->local == 2) {
            Env *b = env_find(c.env, d);
            if (!b) break;
            Env *env = b->ctx_env;
            for (int i = 0; i < e->n; i++) {
                if (d->params[i]->arity) env = mk_binding(d->params[i], eval_operator_arg(e->args[i], c), env);
                else env = mk_arg(d->params[i], e->args[i], c, env);
            }
            return gen(d->body, (Ctx){env, b->ctx_inst}, k);
        }
        if (def_level(d) == 0 && e->n == 0) break;
        Env *env = NULL;
        for (int i = 0; i < e->n; i++) {
            if (d->params[i]->arity) env = mk_binding(d->params[i], eval_operator_arg(e->args[i], c), env);
            else env = mk_arg(d->params[i], e->args[i], c, env);
        }
        deeper();
        int r = gen(d->body, (Ctx){env, c.inst}, k);
        eval_depth--;
        return r;
    }
    case K_PARAMCALL: {
        /* A constant operator that stands for a definition is a call of it. */
        if (e->def->kind != D_CONST) break;
        Ctx at = {NULL, NULL};
        Def *d = const_operator(e->def, c, &at);
        if (!d || d->kind != D_OP || d->native || d->local == 2 || d->nparams != e->n) break;
        Env *env = NULL;
        for (int i = 0; i < e->n; i++) {
            if (d->params[i]->arity) env = mk_binding(d->params[i], eval_operator_arg(e->args[i], c), env);
            else env = mk_arg(d->params[i], e->args[i], c, env);
        }
        return gen(d->body, (Ctx){env, at.inst}, k);
    }
    case K_INSTCALL: {
        if (e->def->kind != D_OP || e->def->native) break;
        /* As a definition of the module, one TLC computes as a constant. */
        int plain = e->n == 0;
        for (int i = 0; i < e->ndefs && plain; i++) plain = e->defs[i]->nparams == 0;
        if (plain && expr_level(e) == 0) break;
        return gen(e->def->body, instance_ctx(e, c), k);
    }
    case K_BOUND: {
        /* A parameter stands for its argument, which may be an action. */
        Env *b = env_find(c.env, e->def);
        if (!b || !b->e) break;
        return gen(b->e, (Ctx){b->ctx_env, b->ctx_inst}, k);
    }
    case K_EQ: {
        int var = assignable(e->args[0], c);
        if (var >= 0 && !gen_target[var]) return gen_assign(var, eval(e->args[1], c), k);
        break;
    }
    case K_IN: {
        int var = assignable(e->args[0], c);
        if (var >= 0 && !gen_target[var]) {
            Value *s = eval(e->args[1], c);
            if (!is_set(s)) fail_at(e, "Attempted to check if a value is an element of the non-set:\n%s", pretty(s));
            /* TLC enumerates any set it can, failing there for an infinite one. */
            if (user_set(s)) fail_at(e, "In computing %s states, the right side of \\IN is not enumerable.", gen_init ? "initial" : "next");
            if (s->kind == V_LAZY && s->lazy == L_FILTER && !s->u.l.b) {   /* as TLC, one element at a time */
                Value *base = filter_base(s);
                for (int i = 0; i < base->n; i++)
                    if (filter_member_test(s, base->u.set.elems[i]) == TRUEV && gen_assign(var, base->u.set.elems[i], k)) return 1;
                return 0;
            }
            s = elements(s);
            for (int i = 0; i < s->n; i++) if (gen_assign(var, s->u.set.elems[i], k)) return 1;
            return 0;
        }
        break;
    }
    case X_APPLY: {
        /* As TLC: a lazy function applied is its body, part of the
           action; TLC computes initial states without keeping it lazy. */
        Value *key, *f = apply_parts(e, c, !gen_init, &key);
        Ctx at = c;
        Expr *body = !f ? fcndef_bind(e->args[0]->def, key, &at, e->args[0])
                   : f->kind == V_LAMBDA && !f->n ? lambda_bind(f, key, &at, e->args[0]) : NULL;
        if (!body) {
            Value *v = fcn_apply(f, key, e);
            if (v->kind != V_BOOL) not_boolean(e, v);
            return v->u.i ? gen_cont(k) : 0;
        }
        Value *saved = at_value;
        if (f) at_value = f->u.l.b;
        int found = gen(body, at, k);
        at_value = saved;
        return found;
    }
    case K_UNCHANGED:
        return gen_unchanged(e->args[0], c, k);
    case X_SQACT: {
        if (gen(e->args[0], c, k)) return 1;
        return gen_unchanged(e->args[1], c, k);
    }
    case X_ANGLEACT: {
        Expr *u = NEW(Expr);
        *u = *e;
        u->kind = K_UNCHANGED;
        u->n = 1;
        u->args = NEWN(Expr *, 1);
        u->args[0] = e->args[1];
        Expr *not = NEW(Expr);
        *not = *e;
        not->kind = K_NOT;
        not->n = 1;
        not->args = NEWN(Expr *, 1);
        not->args[0] = u;
        return gen(e->args[0], c, mk_cont(not, c, k));
    }
    case K_NOT:
        /* The step of <<A>>_v changes v. */
        if (e->args[0]->kind != K_UNCHANGED) break;
        return unchanged_now(e->args[0]->args[0], c) ? 0 : gen_cont(k);
    default:
        break;
    }
    Value *v = eval(e, c);
    if (v->kind != V_BOOL) not_boolean(e, v);
    return v->u.i ? gen_cont(k) : 0;
}

/* Whether action a has a successor from the current state. */
static int enabled_found;
static int enabled_emit(void) {
    enabled_found = 1;
    return 1;
}

static int enabled(Expr *a, Ctx c) {
    Value **saved_target = gen_target, **saved_next = next_state;
    int saved_init = gen_init, saved_primed = primed;
    int (*saved_emit)(void) = gen_emit;
    int saved_names = nenabled_names;
    Value **fresh = NEWN(Value *, nvars + 16);
    gen_target = next_state = fresh;
    gen_init = 0;
    primed = 0;
    gen_emit = enabled_emit;
    enabled_found = 0;
    Inst *mark = NEW(Inst);
    mark->def = &enabled_mark;
    mark->parent = c.inst;
    c.inst = mark;
    gen(a, c, NULL);
    nenabled_names = saved_names;
    int found = enabled_found;
    gen_target = saved_target;
    next_state = saved_next;
    gen_init = saved_init;
    primed = saved_primed;
    gen_emit = saved_emit;
    return found;
}

/* ---------------------------------------------------------------- actions */

/* The next-state action split at its disjunctions, constant existential
   quantifiers, and the definitions it uses; each part is named by the
   definition it belongs to and located by its expression (§11.2). */
typedef struct { Expr *e; Ctx c; int name; Expr *loc; } Action;

static Action *actions;
static int nactions;

static void add_action(Expr *e, Ctx c, int name, Expr *loc) {
    actions = xrealloc(actions, sizeof(Action) * (size_t)(nactions + 1));
    actions[nactions].e = e;
    actions[nactions].c = c;
    actions[nactions].name = name;
    actions[nactions].loc = loc;
    nactions++;
}

typedef struct { Expr *body; int name; } SplitData;

static void split_action(Expr *e, Ctx c, int name);

static int split_fn(Ctx c, void *data) {
    SplitData *s = data;
    split_action(s->body, c, s->name);
    return 0;
}

static int bounds_constant(Expr *e) {
    for (int i = 0; i < e->nb; i++) if (!e->bounds[i].set || expr_level(e->bounds[i].set) > 0) return 0;
    return 1;
}

static void split_action(Expr *e, Ctx c, int name) {
    switch (e->kind) {
    case K_OR:
        for (int i = 0; i < e->n; i++) split_action(e->args[i], c, name);
        return;
    case X_EXISTS:
        if (bounds_constant(e)) {
            SplitData s = {e->args[0], name};
            for_bindings(e->bounds, e->nb, c, split_fn, &s);
            return;
        }
        break;
    case X_LET: {
        Env *env = c.env;
        for (int i = 0; i < e->ndefs; i++) env = mk_thunk(e->defs[i], e->defs[i]->body, c, env);
        for (Env *b = env; b != c.env; b = b->next) b->ctx_env = env;
        split_action(e->args[0], (Ctx){env, c.inst}, name);
        return;
    }
    case K_CALL: {
        Def *d = e->def->replacement ? e->def->replacement : e->def;
        if (d->kind != D_OP || d->native || d->local == 2 || d->nparams != e->n) break;
        Env *env = NULL;
        for (int i = 0; i < e->n; i++) {
            if (d->params[i]->arity) env = mk_binding(d->params[i], eval_operator_arg(e->args[i], c), env);
            else env = mk_arg(d->params[i], e->args[i], c, env);
        }
        deeper();
        split_action(d->body, (Ctx){env, c.inst}, d->name);
        eval_depth--;
        return;
    }
    case K_INSTCALL: {
        if (e->def->kind != D_OP || e->def->native) break;
        /* As TLC, a definition seen through an instance that substitutes is
           one action, named I!A and located at the INSTANCE statement. */
        Buf b = {0};
        Def *in = NULL;
        for (int i = 0; i < e->ndefs; i++) {
            if (!in && e->defs[i]->nsubst) in = e->defs[i];
            if (e->defs[i]->name) buf_printf(&b, "%s!", S(e->defs[i]->name));
        }
        buf_puts(&b, S(e->def->name));
        int qualified = intern(b.s);
        free(b.s);
        if (in) { add_action(e, c, qualified, in->where); return; }
        split_action(e->def->body, instance_ctx(e, c), qualified);
        return;
    }
    default:
        break;
    }
    add_action(e, c, name, e);
}

/* ---------------------------------------------------------------- states */

/* The fingerprint of a value hashes its canonical form, the same for
   equal values. An atom mixes its kind (tag 1 to 4) and payload; a set sums
   its elements' hashes and a function its pairs', a function on 1..n
   being a tuple, so that a function with one pair changed differs by one
   term (fcn_update, Append). A set or function keeps its sum. The mixing
   is wyhash's: the halves of a full product, folded. */
static inline uint64_t wymix(uint64_t a, uint64_t b) {
    __uint128_t m = (__uint128_t)a * b;
    return (uint64_t)m ^ (uint64_t)(m >> 64);
}

static inline uint64_t fmix(uint64_t h) { return wymix(h ^ 0xa0761d6478bd642fULL, 0xe7037ed1a0b428dbULL); }

static uint64_t value_hash(Value *v);

static inline uint64_t atom_word(int kind, int64_t x) { return (uint64_t)(kind + 1) | (uint64_t)x << 8; }

static inline uint64_t elem_hash(Value *v) { return v->kind <= V_MODEL ? fmix(atom_word(v->kind, v->u.i)) : value_hash(v); }

/* The hashes of the first indices of tuples, 1 to 64. */
static uint64_t index_hash[64];

/* The hash of pair i of f, with value val: the key's hash and the value's,
   or an atom's word, which the pair's mixing spreads. */
static inline uint64_t pair_hash(Value *f, int i, Value *val) {
    uint64_t k = f->kind != V_TUPLE ? elem_hash(f->u.fcn.keys[i]) : i < 64 ? index_hash[i] : fmix(atom_word(V_INT, i + 1));
    uint64_t x = val->kind <= V_MODEL ? atom_word(val->kind, val->u.i) : value_hash(val);
    return wymix(k ^ 0x8ebc6af09c88c6e3ULL, x ^ 0x589965cc75374cc3ULL);
}

/* A lazy function hashes as the function it is; out of value_hash's way. */
static __attribute__((cold, noinline)) uint64_t hash_lambda(Value *v) {
    Value *f = realize(v);
    if (f->kind == V_LAMBDA) count_infinite(f->u.l.a);
    return value_hash(f);
}

static uint64_t value_hash(Value *v) {
    uint64_t sum = 0;
    switch (v->kind) {
    case V_BOOL: case V_INT: case V_STR: case V_MODEL: return elem_hash(v);
    case V_INTERVAL: {
        uint64_t n = (uint64_t)interval_size(v);
        for (int64_t i = v->u.range.lo; i <= v->u.range.hi; i++) sum += fmix(atom_word(V_INT, i));
        return fmix(sum ^ (0x05 | n << 8));
    }
    case V_SET: case V_LAZY: {
        if (user_set(v)) fail("TLC has found a state in which the value of a variable contains %s", pretty(v));
        Value *set = enumerate(union_cache(v));
        if (!(sum = set->u.set.hash)) {
            fp_enter(set->u.set.source);
            for (int i = 0; i < set->n; i++) sum += elem_hash(set->u.set.elems[i]);
            fp_leave(set->u.set.source);
            set->u.set.hash = sum;
        }
        return fmix(sum ^ (0x05 | (uint64_t)set->n << 8));
    }
    case V_LAMBDA: return hash_lambda(v);
    case V_TUPLE: case V_FCN:
        if (v->made >= MADE_LAMBDA) v->made = MADE_PLAIN;   /* TLC normalizes what it fingerprints */
        if (!(sum = v->u.fcn.hash)) {
            fp_enter(v->u.fcn.source);
            for (int i = 0; i < v->n; i++) sum += pair_hash(v, i, v->u.fcn.vals[i]);
            fp_leave(v->u.fcn.source);
            v->u.fcn.hash = sum;
        }
        return fmix(sum ^ (0x06 | (uint64_t)v->n << 8));
    default:
        fail("Attempted to fingerprint an operator.");
    }
}

/* A state is stored as the bytes of its values, which keep how a value
   prints. */
static void put32(Buf *b, int64_t v) { int32_t x = (int32_t)v; buf_add(b, (char *)&x, 4); }

/* A tag and its word. */
static void put_tagged(Buf *b, char tag, int64_t v) {
    char w[5] = {tag};
    int32_t x = (int32_t)v;
    memcpy(w + 1, &x, 4);
    buf_add(b, w, 5);
}

static void encode(Buf *b, Value *v) {
    if (v->kind == V_LAMBDA) v = realize(v);
    switch (v->kind) {
    case V_BOOL: buf_add(b, v->u.i ? "b\1" : "b\0", 2); return;
    case V_INT: put_tagged(b, 'i', v->u.i); return;
    case V_STR: put_tagged(b, 's', v->u.i); return;
    case V_MODEL: put_tagged(b, 'm', v->u.i); return;
    case V_INTERVAL: put_tagged(b, 'I', v->u.range.lo); put32(b, v->u.range.hi); return;
    case V_SET: case V_LAZY: {
        Value *set = enumerate(v);
        put_tagged(b, 'S', set->n);
        for (int i = 0; i < set->n; i++) encode(b, set->u.set.elems[i]);
        return;
    }
    case V_TUPLE: case V_FCN: {
        if (v->kind == V_FCN && v->n == 0) { put_tagged(b, 'T', 0); return; }
        put_tagged(b, v->kind == V_TUPLE ? 'T' : v->made == MADE_RECORD ? 'R' : 'F', v->n);
        for (int i = 0; i < v->n; i++) {
            if (v->kind == V_FCN) encode(b, v->u.fcn.keys[i]);
            encode(b, v->u.fcn.vals[i]);
        }
        return;
    }
    default:
        fail("Attempted to fingerprint an operator.");
    }
}

static int32_t get32(const char **p) { int32_t x; memcpy(&x, *p, 4); *p += 4; return x; }

/* A value of a state, with its hash: the successors that keep or update it
   need not hash it again. */
static Value *decode(const char **p) {
    char tag = *(*p)++;
    uint64_t sum = 0;
    switch (tag) {
    case 'b': return BOOLV(*(*p)++);
    case 'i': return mk_int(get32(p));
    case 's': return mk_str(get32(p));
    case 'm': return mk_model(get32(p));
    case 'I': { int32_t lo = get32(p); int32_t hi = get32(p); return mk_interval(lo, hi); }
    case 'S': {
        int n = get32(p);
        Value *s = mk_set(n);
        for (int i = 0; i < n; i++) sum += elem_hash(s->u.set.elems[i] = decode(p));
        s->u.set.hash = sum;
        return s;
    }
    case 'T': {
        int n = get32(p);
        Value *t = mk_tuple(n);
        for (int i = 0; i < n; i++) sum += pair_hash(t, i, t->u.fcn.vals[i] = decode(p));
        t->u.fcn.hash = sum;
        return t;
    }
    default: {
        int n = get32(p);
        Value *f = mk_fcn(n);
        for (int i = 0; i < n; i++) {
            f->u.fcn.keys[i] = decode(p);
            sum += pair_hash(f, i, f->u.fcn.vals[i] = decode(p));
        }
        f->u.fcn.hash = sum;
        if (tag == 'R') f->made = MADE_RECORD;
        return f;
    }
    }
}

typedef struct { uint64_t fp; size_t off; int len; int parent; int action; int depth; int *edges; int nedges, capedges; } StateRec;

static StateRec *states;
static int nstates, capstates;
static Buf store;
/* The fingerprint set: open addressing on the fingerprint, which each
   slot keeps beside its state's index (0 is an empty slot). */
typedef struct { uint64_t fp; int state; } FpSlot;

static FpSlot *fp_table;
static int fp_size;

static int fp_find(uint64_t fp) {
    if (!fp_size) return -1;
    for (int j = (int)(fp & (uint64_t)(fp_size - 1)); fp_table[j].state; j = (j + 1) & (fp_size - 1))
        if (fp_table[j].fp == fp) return fp_table[j].state - 1;
    return -1;
}

static void fp_put(FpSlot *table, int size, uint64_t fp, int idx) {
    int j = (int)(fp & (uint64_t)(size - 1));
    while (table[j].state) j = (j + 1) & (size - 1);
    table[j].fp = fp;
    table[j].state = idx + 1;
}

static char *fp_region;       /* shared memory the tables are taken from, in turn */
static size_t fp_region_used, fp_region_size;

static void fp_insert(int idx) {
    if ((nstates + 1) * 2 >= fp_size) {
        int size = fp_size ? fp_size * 2 : 1 << 16;
        size_t bytes = sizeof(FpSlot) * (size_t)size;
        FpSlot *table;
        if (fp_region) {
            if (fp_region_used + bytes > fp_region_size) { fputs("tpp: too many states\n", stderr); exit(TPP_ERROR); }
            table = (FpSlot *)(fp_region + fp_region_used);
            fp_region_used += bytes;
        } else {
            table = calloc((size_t)size, sizeof(FpSlot));
        }
        for (int i = 0; i < fp_size; i++) if (fp_table[i].state) fp_put(table, size, fp_table[i].fp, fp_table[i].state - 1);
        if (!fp_region) free(fp_table);
        fp_table = table;
        fp_size = size;
    }
    fp_put(fp_table, fp_size, states[idx].fp, idx);
}

static Value **decode_state(int idx) {
    Value **st = NEWN(Value *, nvars);
    const char *p = store.s + states[idx].off;
    for (int i = 0; i < nvars; i++) st[i] = decode(&p);
    return st;
}

/* ---------------------------------------------------------------- the model */

typedef struct { Def *def; Expr *expr; int name; } Check;

static Expr *init_expr, *next_expr;
static Ctx root_ctx;
static Check *invariants, *implied_inits, *constraints, *action_constraints, *action_props, *temporal_props;
static int ninvariants, nimplied_inits, nconstraints, naction_constraints, naction_props, ntemporal_props;
static Expr *view_expr;
static Value **symmetry;     /* permutations of model values */
static int nsymmetry;
static int check_deadlock = 1;
static Module *root;
static Expr **fairness;       /* WF and SF conjuncts of the specification */
static int nfairness;
static Expr *spec_subscript;

static void add_check(Check **list, int *n, Def *d, Expr *e, int name) {
    *list = xrealloc(*list, sizeof(Check) * (size_t)(*n + 1));
    (*list)[*n].def = d;
    (*list)[*n].expr = e;
    (*list)[*n].name = name;
    (*n)++;
}

/* The model value a permutation maps v to; any other value is itself. */
static Value *permute(Value *v, Value *perm) {
    if (v->kind == V_MODEL)
        for (int i = 0; i < perm->n; i++) {
            Value *k = perm->u.fcn.keys[i];
            if (k->kind == V_MODEL && k->u.i == v->u.i) return perm->u.fcn.vals[i];
        }
    return v;
}

/* v with each model value mapped by perm. */
static Value *permute_value(Value *v, Value *perm) {
    switch (v->kind) {
    case V_MODEL: return permute(v, perm);
    case V_SET: case V_LAZY: {
        if (v->kind == V_LAZY && set_is_infinite(v)) return v;
        Value *s = enumerate(v);
        List l = {0};
        for (int i = 0; i < s->n; i++) list_add(&l, permute_value(s->u.set.elems[i], perm));
        return list_set(&l);
    }
    case V_TUPLE: {
        Value *t = mk_tuple(v->n);
        for (int i = 0; i < v->n; i++) t->u.fcn.vals[i] = permute_value(v->u.fcn.vals[i], perm);
        return t;
    }
    case V_FCN: {
        List keys = {0}, vals = {0};
        for (int i = 0; i < v->n; i++) {
            list_add(&keys, permute_value(v->u.fcn.keys[i], perm));
            list_add(&vals, permute_value(v->u.fcn.vals[i], perm));
        }
        Value *f = mk_fcn_from(&keys, &vals);
        f->made = v->made;
        return f;
    }
    default:
        return v;
    }
}

/* x mapped by q, then by p, as a permutation without its fixed points. */
static Value *perm_compose(Value *p, Value *q) {
    List keys = {0}, vals = {0};
    Value *dom[2] = {q, p};
    for (int d = 0; d < 2; d++)
        for (int i = 0; i < dom[d]->n; i++) {
            Value *x = fcn_key(dom[d], i), *y = permute(permute(x, q), p);
            int seen = veq(x, y);
            for (int k = 0; k < keys.n && !seen; k++) seen = veq(keys.v[k], x);
            if (!seen) { list_add(&keys, x); list_add(&vals, y); }
        }
    return mk_fcn_from(&keys, &vals);
}

/* The group the permutations generate, as TLC takes a SYMMETRY: each
   composition of them, without fixed points so that equal ones are
   equal values. */
static Value *permutation_group(Value *generators) {
    List g = {0};
    size_t size = 64;
    int *table = calloc(size, sizeof(int));   /* indices into g, plus one, by hash */
    Value *identity = mk_fcn_from(&(List){0}, &(List){0});
    for (int i = -1; i < g.n; i++) {
        for (int j = 0; j < generators->n; j++) {
            Value *c = perm_compose(generators->u.set.elems[j], i < 0 ? identity : g.v[i]);
            size_t h = (size_t)value_hash(c) & (size - 1);
            while (table[h] && !veq(g.v[table[h] - 1], c)) h = (h + 1) & (size - 1);
            if (table[h]) continue;
            list_add(&g, c);
            table[h] = g.n;
            if ((size_t)g.n * 2 > size) {   /* grow, rehashing */
                free(table);
                size *= 2;
                table = calloc(size, sizeof(int));
                for (int k = 0; k < g.n; k++) {
                    size_t t = (size_t)value_hash(g.v[k]) & (size - 1);
                    while (table[t]) t = (t + 1) & (size - 1);
                    table[t] = k + 1;
                }
            }
        }
    }
    free(table);
    return list_set(&g);
}

/* The model values the symmetry moves, and the hashes of their images
   under each permutation p: as elements at [p * m + j], as words of a
   function's values after them. */
static Value **sym_models;
static int nsym_models;
static uint64_t *sym_elem, *sym_word;

static void symmetry_tables(void) {
    List m = {0};
    for (int p = 0; p < nsymmetry; p++)
        for (int i = 0; i < symmetry[p]->n; i++) {
            Value *k = symmetry[p]->u.fcn.keys[i];
            int seen = 0;
            for (int j = 0; j < m.n && !seen; j++) seen = veq(m.v[j], k);
            if (!seen) list_add(&m, k);
        }
    sym_models = m.v;
    nsym_models = m.n;
    sym_elem = xmalloc(sizeof(uint64_t) * 2 * (size_t)nsymmetry * (size_t)(m.n + 1));
    sym_word = sym_elem + (size_t)nsymmetry * (size_t)m.n;
    for (int p = 0; p < nsymmetry; p++)
        for (int j = 0; j < m.n; j++) {
            Value *w = permute(m.v[j], symmetry[p]);
            sym_elem[p * m.n + j] = elem_hash(w);
            sym_word[p * m.n + j] = atom_word(w->kind, w->u.i);
        }
}

/* Space for permuted_hashes at each depth: three hashes per permutation. */
static uint64_t **sym_bufs;
static int nsym_bufs;

static uint64_t *sym_buf(int depth) {
    if (depth == nsym_bufs) {
        sym_bufs = xrealloc(sym_bufs, sizeof(uint64_t *) * (size_t)++nsym_bufs);
        sym_bufs[depth] = xmalloc(sizeof(uint64_t) * 3 * (size_t)nsymmetry);
    }
    return sym_bufs[depth];
}

/* Whether a permutation of the symmetry moves a model value in v; if so,
   h[p] is value_hash(permute(v, symmetry[p])) for each p, computed without
   building it: a permutation is one to one, and a set's or function's
   hash is the sum of its parts'. With atom, an atom's h[p] is its word,
   as a function's value is hashed. */
static int permuted_hashes(Value *v, uint64_t *h, int atom, int depth) {
    int n = nsymmetry, moved = 0;
    if (v->kind <= V_MODEL) {
        int j = 0, m = nsym_models;
        if (v->kind == V_MODEL) while (j < m && sym_models[j]->u.i != v->u.i) j++;
        if (v->kind != V_MODEL || j == m) return 0;
        uint64_t *t = atom ? sym_word : sym_elem;
        for (int p = 0; p < n; p++) h[p] = t[p * m + j];
        return 1;
    }
    uint64_t *sums = sym_buf(depth), *k = sums + n, *x = k + n, fixed = 0, tag;
    if (v->kind == V_SET || (v->kind == V_LAZY && !set_is_infinite(v))) {
        Value *set = enumerate(v);
        for (int i = 0; i < set->n; i++) {
            Value *e = set->u.set.elems[i];
            if (!permuted_hashes(e, k, 0, depth + 1)) { fixed += elem_hash(e); continue; }
            if (!moved) memset(sums, 0, sizeof(uint64_t) * (size_t)n);
            moved = 1;
            for (int p = 0; p < n; p++) sums[p] += k[p];
        }
        tag = 0x05 | (uint64_t)set->n << 8;
    } else if (v->kind == V_TUPLE || v->kind == V_FCN) {
        for (int i = 0; i < v->n; i++) {
            Value *val = v->u.fcn.vals[i];
            int km = v->kind == V_FCN && permuted_hashes(v->u.fcn.keys[i], k, 0, depth + 1);
            int vm = permuted_hashes(val, x, 1, depth + 1);
            if (!km && !vm) { fixed += pair_hash(v, i, val); continue; }
            uint64_t k0 = km ? 0 : v->kind == V_FCN ? elem_hash(v->u.fcn.keys[i]) : i < 64 ? index_hash[i] : fmix(atom_word(V_INT, i + 1));
            uint64_t x0 = vm ? 0 : val->kind <= V_MODEL ? atom_word(val->kind, val->u.i) : value_hash(val);
            if (!moved) memset(sums, 0, sizeof(uint64_t) * (size_t)n);
            moved = 1;
            for (int p = 0; p < n; p++)
                sums[p] += wymix((km ? k[p] : k0) ^ 0x8ebc6af09c88c6e3ULL, (vm ? x[p] : x0) ^ 0x589965cc75374cc3ULL);
        }
        tag = 0x06 | (uint64_t)v->n << 8;
    } else {
        return 0;   /* an interval, or a value hashed as it is */
    }
    if (moved) for (int p = 0; p < n; p++) h[p] = fmix((sums[p] + fixed) ^ tag);
    return moved;
}

/* As TLC takes a state under symmetry: the least of its permutations,
   comparing the variables' values in turn. */
static Value **least_permutation(Value **st) {
    Value **best = st;
    for (int p = 0; p < nsymmetry; p++) {
        Value **t = NEWN(Value *, nvars);
        int c = 0;
        for (int i = 0; i < nvars; i++) {
            t[i] = permute_value(st[i], symmetry[p]);
            if (!c) c = vcmp(t[i], best[i]);
        }
        if (c < 0) best = t;
    }
    return best;
}

static uint64_t fingerprint(Value **st) {
    if (view_expr) {
        Value **saved = cur_state;
        cur_state = nsymmetry ? least_permutation(st) : st;
        Value *v = eval(view_expr, root_ctx);
        cur_state = saved;
        return value_hash(v);
    }
    uint64_t h = 0;
    if (!nsymmetry) {
        for (int i = 0; i < nvars; i++) h = fmix(h + elem_hash(st[i]));
        return h;
    }
    /* The least fingerprint of the state's permutations. A successor shares
       most of its values with its state's: each variable's last value and
       its hashes under the permutations, h[p] the same for all if unmoved,
       are kept. */
    static uint64_t *fps, *vh;
    static Value **last;
    static unsigned *last_resets;
    if (!fps) {
        fps = xmalloc(sizeof(uint64_t) * (size_t)nsymmetry);
        vh = xmalloc(sizeof(uint64_t) * (size_t)nsymmetry * (size_t)nvars);
        last = calloc((size_t)nvars, sizeof(Value *));
        last_resets = calloc((size_t)nvars, sizeof(unsigned));
    }
    memset(fps, 0, sizeof(uint64_t) * (size_t)nsymmetry);
    for (int i = 0; i < nvars; i++) {
        uint64_t *h = vh + (size_t)i * (size_t)nsymmetry;
        if (st[i] != last[i] || last_resets[i] != scratch_resets) {
            if (!permuted_hashes(st[i], h, 0, 0)) {
                uint64_t same = elem_hash(st[i]);
                for (int p = 0; p < nsymmetry; p++) h[p] = same;
            }
            last[i] = st[i];
            last_resets[i] = scratch_resets;
        }
        for (int p = 0; p < nsymmetry; p++) fps[p] = fmix(fps[p] + h[p]);
    }
    uint64_t best = fps[0];
    for (int p = 1; p < nsymmetry; p++) if (fps[p] < best) best = fps[p];
    return best;
}

static int nworkers = 1;   /* processes expanding states; with more than one, the store is shared */

/* A new state whose encoding ends the store. */
static int add_record(size_t off, uint64_t fp, int parent, int action, int depth) {
    if (nstates == capstates) {
        if (nworkers > 1) { fputs("tpp: too many states\n", stderr); exit(TPP_ERROR); }
        capstates = capstates ? capstates * 2 : 1024;
        states = xrealloc(states, sizeof(StateRec) * (size_t)capstates);
    }
    StateRec *r = &states[nstates];
    r->fp = fp;
    r->off = off;
    r->len = (int)(store.n - off);
    r->parent = parent;
    r->action = action;
    r->depth = depth;
    r->edges = NULL;
    r->nedges = r->capedges = 0;
    fp_insert(nstates);
    return nstates++;
}

static void store_add(const char *bytes, size_t n) {
    if (nworkers > 1 && store.n + n + 1 > store.cap) { fputs("tpp: too many states\n", stderr); exit(TPP_ERROR); }
    buf_add(&store, bytes, n);
}

static int add_state(Value **st, uint64_t fp, int parent, int action, int depth) {
    size_t off = store.n;
    for (int i = 0; i < nvars; i++) encode(&store, st[i]);
    return add_record(off, fp, parent, action, depth);
}

/* ---------------------------------------------------------------- reporting */

static long generated;
static int max_depth = 1;
static int expanded;          /* states taken off the queue */

static void fprint_state(FILE *f, Value **st) {
    if (nvars == 1) {
        fprintf(f, "%s = %s\n", S(vars[0]->name), st[0] ? pretty(st[0]) : "null");
        return;
    }
    for (int i = 0; i < nvars; i++) fprintf(f, "/\\ %s = %s\n", S(vars[i]->name), st[i] ? pretty(st[i]) : "null");
}

static void print_state(Value **st) { fprint_state(stdout, st); }

static int view_trace;   /* -view */

/* With -view, a reported state as its view's value: returns whether it
   printed it. */
static int print_shown(Value **st) {
    if (!view_trace || !view_expr) return 0;
    Value **saved = cur_state;
    cur_state = st;
    printf("%s\n", pretty(eval(view_expr, root_ctx)));
    cur_state = saved;
    return 1;
}

static void action_label(int action) {
    if (action < 0) { printf("<Initial predicate>"); return; }
    Action *a = &actions[action];
    Expr *l = a->loc;
    printf("<%s line %d, col %d to line %d, col %d of module %s>", S(a->name), l->l1, l->c1, l->l2, l->c2, S(l->mod->name));
}

static int cur_action;

/* TLC rebuilds the states of a trace from their fingerprints by evaluating
   the spec again: all of Init for the first state, then each action in turn
   until one yields the next state. Side effects such as Print happen
   again, so tpp replays the same evaluations for the same output. The
   state found is the first with the fingerprint, which under a view or
   symmetry may be another than the one stored. */
static Value **replay_target;
static uint64_t replay_fp;
static int replay_found;
static Value **replay_match;   /* the state found */
static int replay_checks;     /* also evaluate the properties and invariants, as TLC does replaying Init */
static int check_list(Check *list, int n, Value **st);
static void live_init_lits(void);
static void live_init(Value **st);
static void eval_init_lits(Value **st);
static int live_init_failed;   /* an error in a property on an initial state */

static int replay_emit(void) {
    if (!replay_found) {
        int complete = 1;
        for (int i = 0; i < nvars && complete; i++) complete = gen_target[i] != NULL;
        replay_found = complete && replay_target && fingerprint(gen_target) == replay_fp;
        if (replay_found) {   /* normalized, as a state TLC keeps */
            replay_match = memcpy(NEWN(Value *, nvars), gen_target, sizeof(Value *) * (size_t)nvars);
            for (int i = 0; i < nvars; i++) materialize(replay_match[i]);
        }
    }
    if (replay_checks) {
        if (ntemporal_props) eval_init_lits(gen_target);
        check_list(invariants, ninvariants, gen_target);
    }
    return 0;
}

/* The step from `from`, at level `level` (Init's 0 if from is NULL), to `to`. */
static int replay(Value **from, Value **to, int64_t level) {
    Value **saved_cur = cur_state, **saved_next = next_state, **saved_target = gen_target;
    int (*saved_emit)(void) = gen_emit;
    int saved_init = gen_init;
    int64_t saved_level = cur_level;
    cur_level = level;
    Value **st = NEWN(Value *, nvars);
    /* A successor not completely specified is no state to find. */
    replay_target = to;
    for (int i = 0; i < nvars && replay_target; i++) if (!to[i]) replay_target = NULL;
    if (replay_target) replay_fp = fingerprint(to);
    replay_found = 0;
    replay_match = to;
    gen_target = st;
    gen_emit = replay_emit;
    /* Model checking draws as the search did; in a simulation TLC's main
       thread rebuilds a behavior, drawing from its generator. */
    JRandom *saved_drawing = drawing, saved_model = draw_model;
    Value **saved_from = draw_from;
    int saved_due = draw_due;
    drawing = &draw_model;
    if (!simulating) draw_restart(from);
    if (!from) {
        gen_init = 1;
        cur_state = st;
        next_state = NULL;
        gen(init_expr, root_ctx, NULL);
    }
    int action = -1;
    for (int a = 0; from && a < nactions && !replay_found; a++) {
        gen_init = 0;
        cur_state = from;
        memset(st, 0, sizeof(Value *) * (size_t)nvars);
        next_state = st;
        gen(actions[a].e, actions[a].c, NULL);
        action = a;
    }
    cur_state = saved_cur; next_state = saved_next; gen_target = saved_target;
    gen_emit = saved_emit; gen_init = saved_init;
    cur_level = saved_level;
    drawing = saved_drawing;
    if (!simulating) draw_model = saved_model, draw_from = saved_from, draw_due = saved_due;
    return action;   /* the action that yields `to`, the last tried if none */
}

/* After an evaluation error TLC evaluates Init, or the next-state action
   of the state it failed in, again to locate the error: from == NULL
   replays Init with its checks. The error happens again and ends it. */
static void replay_error(Value **from) {
    sigjmp_buf here, *saved = handler;
    handler = &here;
    if (!sigsetjmp(here, 0)) {
        replay_checks = !from;
        replay(from, NULL, cur_level);
    }
    replay_checks = 0;
    handler = saved;
}

static int diff_trace;   /* -difftrace */

/* Whether a variable keeps its value p as v, as TLC compares them: an
   error comparing, as of a string with a number, is a change. */
static int unchanged(Value *p, Value *v) {
    if (!p || !v) return 0;
    EvalState s = eval_save();
    ErrorState e = error_save();
    sigjmp_buf here, *outer = handler;
    handler = &here;
    int same = !sigsetjmp(here, 0) && veq(p, v);
    handler = outer;
    eval_restore(s);
    error_restore(&e);
    return same;
}

/* State number of a trace, reached by action. With -difftrace, only the
   variables that changed from prev, unless prev is NULL. */
static void print_numbered(int number, int action, Value **st, Value **prev) {
    printf("State %d: ", number);
    action_label(action);
    printf("\n");
    if (print_shown(st)) return;   /* no blank line after a view, as TLC prints it */
    if (diff_trace && prev) {
        for (int i = 0; i < nvars; i++)
            if (!unchanged(prev[i], st[i]))
                printf("%s%s = %s\n", nvars == 1 ? "" : "/\\ ", S(vars[i]->name), st[i] ? pretty(st[i]) : "null");
    } else {
        print_state(st);
    }
    printf("\n");
}

static void print_trace_states(int s1, Value **s2, int action) {
    int n = 0;
    for (int i = s1; i >= 0; i = states[i].parent) n++;
    int *path = NEWN(int, n);
    Value ***st = NEWN(Value **, n);
    int k = n;
    for (int i = s1; i >= 0; i = states[i].parent) path[--k] = i;
    for (int i = 0; i < n; i++) st[i] = decode_state(path[i]);
    /* Each state as TLC rebuilds it, from the one before as rebuilt. */
    for (int i = 0; i + 1 < n; i++) { replay(i ? st[i - 1] : NULL, st[i], i); st[i] = replay_match; }
    /* The last state is printed whole, as TLC prints it. */
    printf("Error: The behavior up to this point is:\n");
    for (int i = 0; i + 1 < n; i++) print_numbered(i + 1, states[path[i]].action, st[i], i ? st[i - 1] : NULL);
    if (n > 1) { replay(st[n - 2], st[n - 1], n - 1); st[n - 1] = replay_match; }
    print_numbered(n, states[s1].action, st[n - 1], s2 && n > 1 ? st[n - 2] : NULL);
    if (s2) {
        replay(st[n - 1], s2, n);
        print_numbered(n + 1, action, s2, NULL);
    }
}

/* The behavior from an initial state to state s1, then the state s2
   reached from it by `action` if s2 is not NULL, as TLC prints it. As
   TLC, an error rebuilding a state ends the behavior there, unreported:
   returns the exit status of that error, 0 if none. */
static int print_trace(int s1, Value **s2, int action) {
    EvalState s = eval_save();
    ErrorState e = error_save();
    int failed = 0;
    sigjmp_buf here;
    handler = &here;
    if (!sigsetjmp(here, 0)) print_trace_states(s1, s2, action);
    else failed = eval_failure_status();
    eval_restore(s);
    error_restore(&e);
    return failed;
}

static void print_counts(int queued) {
    printf("%ld states generated, %d distinct states found, %d states left on queue.\n", generated, nstates, queued);
    printf("The depth of the complete state graph search is %d.\n", generated ? max_depth : 0);
    fflush(stdout);
}

/* TLC's report of a Java exception, up to its message. */
static void print_exception(const char *cls, const char *then) {
    printf("Error: TLC threw an unexpected exception.\n"
           "This was probably caused by an error in the spec or model.\n"
           "See the User Output or TLC Console for clues to what happened.\n"
           "The exception was a %s\n%s", cls, then);
}

static void print_error_message(void) {
    printf("Error: %s", error_message.s ? error_message.s : "");
    if (error_expr && error_expr->mod) {
        Buf b = {0};
        node_text(&b, error_expr);
        printf("\n%s", b.s);
        free(b.s);
    }
    printf("\n");
}

/* ---------------------------------------------------------------- search */

static int expanding = -1;    /* the state whose successors are generated */
static Value **succ;          /* the assignment the generator fills */
static int succ_count;
static int violation;         /* the exit status of the first violation, 0 if none */

/* The check being evaluated, and its state, when an error stops it. */
static Check *error_check;
static Value **error_state;

/* Whether the check an error stopped is one of list. */
static int checking(Check *list, int n) { return error_check && n && error_check >= list && error_check < list + n; }
static int checking_invariant(void) { return checking(invariants, ninvariants); }

/* A check that is no boolean, an error TLC raises outside its evaluator. */
static _Noreturn void not_boolean_check(Check *c) {
    Expr *body = c->def && c->def->body ? c->def->body : c->expr;
    error_reset();
    buf_puts(&error_message, "TLC expected a boolean value, but did not find one. ");
    node_text(&error_message, body);
    error_status = TPP_FAILURE_EVAL;
    stackless = 1;
    raise_error();
}

static int check_list(Check *list, int n, Value **st) {
    Value **saved = cur_state;
    cur_state = st;
    int bad = -1;
    for (int i = 0; i < n && bad < 0; i++) {
        error_check = &list[i];
        error_state = st;
        Value *v = eval(list[i].expr, root_ctx);
        if (v->kind != V_BOOL) {
            cur_state = saved;
            not_boolean_check(&list[i]);
        }
        if (!v->u.i) bad = i;
    }
    error_check = NULL;
    error_state = NULL;
    cur_state = saved;
    return bad;
}

static int check_actions(Check *list, int n, Value **from, Value **to) {
    Value **saved_cur = cur_state, **saved_next = next_state;
    cur_state = from;
    next_state = to;
    int bad = -1;
    for (int i = 0; i < n && bad < 0; i++) {
        error_check = &list[i];
        error_state = to;
        Value *v = eval(list[i].expr, root_ctx);
        if (v->kind != V_BOOL) not_boolean_check(&list[i]);
        if (!v->u.i) bad = i;
    }
    error_check = NULL;
    error_state = NULL;
    cur_state = saved_cur;
    next_state = saved_next;
    return bad;
}

#define STUTTER (-2)

/* An edge of the state graph, kept for liveness checking: target and action. */
static void add_edge(int from, int to, int action) {
    StateRec *r = &states[from];
    for (int i = 0; i < r->nedges; i++) if (r->edges[2 * i] == to) return;
    if (r->nedges == r->capedges) {
        r->capedges = r->capedges ? r->capedges * 2 : 4;
        r->edges = xrealloc(r->edges, sizeof(int) * 2 * (size_t)r->capedges);
    }
    r->edges[2 * r->nedges] = to;
    r->edges[2 * r->nedges + 1] = action;
    r->nedges++;
}

/* The state graph in dot, as TLC's -dump dot writes it: each state as
   it is found, an initial one filled, and each step as it is taken; then
   by depth the initial states and those a step was taken from. */
static FILE *dump_file;
static int dump_actions;
static int *dump_ranked, dump_nranked;

static void dump_rank(int idx) {
    if (dump_nranked && dump_ranked[dump_nranked - 1] == idx) return;
    dump_ranked = xrealloc(dump_ranked, sizeof(int) * (size_t)(dump_nranked + 1));
    dump_ranked[dump_nranked++] = idx;
}

static void dump_node(int idx, Value **st) {
    Buf b = {0};
    for (int i = 0; i < nvars; i++) {
        if (i) buf_puts(&b, "\\n");
        if (nvars > 1) buf_puts(&b, "/\\\\ ");
        buf_printf(&b, "%s = ", S(vars[i]->name));
        for (const char *c = st[i] ? pretty(st[i]) : "null"; *c; c++) {
            if (*c == '\n') { buf_puts(&b, "\\n"); continue; }
            if (*c == '\\' || *c == '"') buf_add(&b, "\\", 1);
            buf_add(&b, c, 1);
        }
    }
    if (states[idx].parent < 0) fprintf(dump_file, "%lld [label=\"%s\",style = filled]\n", (long long)states[idx].fp, b.s);
    else fprintf(dump_file, "%lld [label=\"%s\",tooltip=\"%s\"];\n", (long long)states[idx].fp, b.s, b.s);
    free(b.s);
    if (states[idx].parent < 0) dump_rank(idx);
}

static void dump_edge(int from, int to, int action) {
    if (states[from].parent >= 0) dump_rank(from);
    fprintf(dump_file, "%lld -> %lld [label=\"%s\",color=\"black\",fontcolor=\"black\"];\n", (long long)states[from].fp,
            (long long)states[to].fp, dump_actions ? S(actions[action].name) : "");
}

static void dump_close(void) {
    for (int i = 0; i < dump_nranked; i++) {
        int s = dump_ranked[i];
        if (!i || states[s].depth != states[dump_ranked[i - 1]].depth) fprintf(dump_file, "%s{rank = same; ", i ? "}\n" : "");
        fprintf(dump_file, "%lld;", (long long)states[s].fp);
    }
    fprintf(dump_file, "%s}\n}", dump_nranked ? "}\n" : "");
    fclose(dump_file);
    dump_file = NULL;
    free(dump_ranked);
    dump_ranked = NULL;
    dump_nranked = 0;
}

/* An initial state checked as TLC checks it: the invariants, then the
   properties that are state predicates. Returns the exit status of a
   violation, which it reports, or 0. */
static int keep_going;   /* -continue: a violated invariant or action property is reported, and the search goes on */

static int check_initial(Value **st) {
    int bad = check_list(invariants, ninvariants, st);
    if (bad >= 0) {
        printf("Error: Invariant %s is violated by the initial state:\n", S(invariants[bad].name));
        if (!print_shown(st)) print_state(st);
        if (!keep_going || simulating) return TPP_VIOLATION_SAFETY;   /* TLC's simulator stops */
        printf("\n");
    }
    bad = check_list(implied_inits, nimplied_inits, st);
    if (bad >= 0) {
        printf("Error: Property %s is violated by the initial state:\n", S(implied_inits[bad].name));
        if (!print_shown(st)) print_state(st);
        return TPP_VIOLATION_LIVENESS;
    }
    return 0;
}

static int init_emit(void) {
    for (int i = 0; i < nvars; i++)
        if (!succ[i]) fail("The variable %s was not assigned a value by the initial predicate.", S(vars[i]->name));
    generated++;
    Value **st = NEWN(Value *, nvars);
    memcpy(st, succ, sizeof(Value *) * (size_t)nvars);
    if (check_list(constraints, nconstraints, st) >= 0) {   /* as TLC, checked though not kept */
        violation = check_initial(st);
        return violation != 0;
    }
    error_state = st;
    uint64_t fp = fingerprint(st);
    error_state = NULL;
    if (fp_find(fp) >= 0) return 0;
    int idx = add_state(st, fp, -1, -1, 1);
    if (dump_file) dump_node(idx, st);
    if (ntemporal_props) live_init(st);
    violation = check_initial(st);
    return violation != 0;
}

/* Whether the state a violation reports was new: as TLC, it is not queued. */
static int violation_fresh;

/* Whether a step to st violates an action property, which TLC checks
   for every step. */
static int action_violated(Value **st) {
    int bad = check_actions(action_props, naction_props, cur_state, st);
    if (bad < 0) return 0;
    printf("Error: Action property %s is violated.\n", S(action_props[bad].name));
    print_trace(expanding, st, cur_action);
    if (keep_going) return 0;
    violation = TPP_VIOLATION_LIVENESS;
    return 1;
}

/* Whether a successor is being fingerprinted: TLC does it outside Next. */
static int fingerprinting;

static int next_emit(void) {
    succ_count++;
    generated++;
    int unassigned = 0;
    for (int i = 0; i < nvars; i++) unassigned += !succ[i];
    if (unassigned) {
        printf("Error: Successor state is not completely specified by ");
        if (nactions == 1) printf("the next-state action");
        else printf("action %s of the next-state relation", S(actions[cur_action].name));
        printf(". The following variable%s not defined: ", unassigned == 1 ? " is" : "s are");
        for (int i = 0, k = 0; i < nvars; i++) if (!succ[i]) printf("%s%s", k++ ? ", " : "", S(vars[i]->name));
        printf(".\n\n");
        print_trace(expanding, succ, cur_action);
        violation = TPP_FAILURE_EVAL;
        return 1;
    }
    Value **st = NEWN(Value *, nvars);
    memcpy(st, succ, sizeof(Value *) * (size_t)nvars);
    /* As TLC: a state outside the model is not kept, but its invariants
       are checked each time it is found. */
    if (check_list(constraints, nconstraints, st) >= 0 || check_actions(action_constraints, naction_constraints, cur_state, st) >= 0) {
        int bad = check_list(invariants, ninvariants, st);
        violation_fresh = 0;
        if (bad >= 0) {
            printf("Error: Invariant %s is violated.\n", S(invariants[bad].name));
            print_trace(expanding, st, cur_action);
            if (!keep_going) {
                violation = TPP_VIOLATION_SAFETY;
                return 1;
            }
        }
        return action_violated(st);
    }
    fingerprinting = 1;
    uint64_t fp = fingerprint(st);
    fingerprinting = 0;
    int found = fp_find(fp);
    if (found >= 0 && ntemporal_props) add_edge(expanding, found, cur_action);
    if (found >= 0 && dump_file) dump_edge(expanding, found, cur_action);
    violation_fresh = found < 0;
    if (found < 0) {
        int depth = states[expanding].depth + 1;
        int idx = add_state(st, fp, expanding, cur_action, depth);
        if (dump_file) dump_edge(expanding, idx, cur_action), dump_node(idx, st);
        if (depth > max_depth) max_depth = depth;
        int bad = check_list(invariants, ninvariants, st);
        if (ntemporal_props) add_edge(expanding, idx, cur_action);
        if (bad >= 0) {
            printf("Error: Invariant %s is violated.\n", S(invariants[bad].name));
            print_trace(expanding, st, cur_action);
            if (!keep_going) {
                violation = TPP_VIOLATION_SAFETY;
                return 1;
            }
        }
    }
    if (action_violated(st)) return 1;
    violation_fresh = 0;
    return 0;
}

static time_t started;   /* when checking began, for TLCGet("duration") */

/* TLCGet's counters of the search, -1 for another key. As TLC, a new
   successor joins the queue once checked; a simulation counts none but
   the time. */
static int64_t search_counter(const char *key) {
    if (!strcmp(key, "duration")) return time(NULL) - started;
    int which = !strcmp(key, "distinct") ? 1 : !strcmp(key, "queue") ? 2 : !strcmp(key, "diameter") ? 3 : 0;
    if (!which || simulating) return -1;
    if (in_worker) worker_cannot();   /* the counts are the search's, not a worker's */
    return which == 1 ? nstates : which == 2 ? nstates - expanded - violation_fresh : max_depth;
}

static void collect_fairnesses(void);
static int live_violated(void);
static int live_error(void);
/* The states generated when each state had been expanded, with temporal
   properties: TLC reports an error in them with these counts. */
static long *gen_after;
static int ngen_after;
static void note_expanded(int idx) {
    if (!ntemporal_props) return;
    if (idx >= ngen_after) {
        ngen_after = idx + 1 > 2 * ngen_after ? idx + 1 : 2 * ngen_after;
        gen_after = xrealloc(gen_after, sizeof(long) * (size_t)ngen_after);
    }
    gen_after[idx] = generated;
}

/* Expands state idx: its successors join the search. Returns the exit
   status of a violation it reports, 0 if none. */
static int expand_state(int idx) {
    expanding = idx;
    expanded = idx + 1;
    cur_level = states[idx].depth;
    cur_state = decode_state(idx);
    draw_restart(cur_state);
    succ_count = 0;
    for (int a = 0; a < nactions; a++) {
        cur_action = a;
        memset(succ, 0, sizeof(Value *) * (size_t)nvars);
        next_state = succ;
        gen_target = succ;
        gen(actions[a].e, actions[a].c, NULL);
        if (violation) {
            print_counts(nstates - expanded - (violation != TPP_FAILURE_EVAL && violation_fresh));
            return violation;
        }
    }
    if (succ_count == 0 && check_deadlock) {
        printf("Error: Deadlock reached.\n");
        print_trace(idx, NULL, 0);
        print_counts(nstates - expanded);
        return TPP_VIOLATION_DEADLOCK;
    }
    note_expanded(idx);
    arena_reset(&scratch_arena);
    return 0;
}

/* ---------------------------------------------------------------- workers */

/* Worker processes expand the states of a level in batches. They read the
   store, the state records and the fingerprint set, which live in shared
   memory, and write what they find to their own shared regions: for each
   state, whether it is clean (no error, violation or deadlock to report)
   and its successors in order. A successor not seen as the level started
   is claimed in a table the workers share; the worker that claims it
   checks the invariants and keeps its encoding, once. The search merges
   the batches in order as they are done, taking a new state's encoding
   from its claim; states it adds meanwhile are in a fingerprint table the
   workers do not read, so they claim them as new. It
   expands a state that is not clean, or that is the first to reach a
   state whose invariants failed, itself, so it reports exactly what one
   process would. */
#define PARALLEL_LEVEL 256    /* smaller levels are expanded by the search alone */
#define BATCH 32
#define CLAIM_CHUNK 64

/* A successor: the state it is if seen as the level started, NEW with
   its claim, or CONSTRAINED. */
typedef struct { uint64_t fp; int action; int seen; uint32_t claim; } Found;
enum { NEW = -1, CONSTRAINED = -2 };
typedef struct { int worker; size_t off; atomic_int done; } Batch;
typedef struct { _Atomic uint64_t key; atomic_int state; int len; uint64_t loc; int merged; } Claim;
enum { CLAIMED, SETTLED, FAILED };    /* a claim's state: its invariants unchecked, held, or not */

typedef struct {
    atomic_int next;          /* the next batch to take */
    int lo, hi, nbatches;     /* the level */
    FpSlot *fp_table;         /* the fingerprint set as the level starts */
    int fp_size;
    size_t nclaims;           /* the claim table's size, a power of two */
    atomic_size_t claimed;
} Control;

static Control *control;
static Batch *batches;
static Claim *claims;
static char **results, **encodings;   /* each worker's regions */
static int (*to_worker)[2], (*from_worker)[2];
static pid_t *worker_pids;
static int workers_running, forked_strs;
static double growth = 1;      /* new states per state expanded, on the last level */
static size_t max_claims, results_size, encodings_size;

/* What a worker's successors may fill of its results, the headers of a
   batch kept. */
static size_t results_room(void) { return results_size - BATCH * 2 * sizeof(int); }

static int w_id;               /* in a worker: its number */
static size_t w_n, w_encoded;  /* bytes used of its regions */
static size_t w_budget;        /* claims it may still make this level */
static int w_clean, w_found;
static Buf w_bytes;

#ifndef MAP_NORESERVE
#define MAP_NORESERVE 0
#endif

static void *shared(size_t bytes) {
    void *p = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON | MAP_NORESERVE, -1, 0);
    if (p == MAP_FAILED) { fputs("tpp: cannot reserve shared memory for workers\n", stderr); exit(TPP_ERROR); }
    return p;
}

/* Puts the store, the state records and the fingerprint set in shared
   memory. Linux and macOS reserve it as it is touched; Windows commits all
   Cygwin maps, so there every region is halved until all fit in a quarter
   of the memory. */
static void share_search(void) {
    int shift = 0;
#ifdef __CYGWIN__
    size_t memory = (size_t)sysconf(_SC_PHYS_PAGES) * (size_t)sysconf(_SC_PAGESIZE);
    size_t all = ((size_t)1 << 37) + sizeof(StateRec) * ((size_t)1 << 29) + ((size_t)1 << 36) +
                 sizeof(Batch) * ((size_t)1 << 29) / BATCH + sizeof(Claim) * ((size_t)1 << 27) +
                 (size_t)nworkers * (((size_t)1 << 34) + ((size_t)1 << 36));
    while ((all >> shift) > memory / 4) shift++;
#endif
    store.cap = ((size_t)1 << 37) >> shift;
    store.s = shared(store.cap);
    capstates = (1 << 29) >> shift;
    states = shared(sizeof(StateRec) * (size_t)capstates);
    fp_region_size = ((size_t)1 << 36) >> shift;
    fp_region = shared(fp_region_size);
    control = shared(sizeof(Control));
    batches = shared(sizeof(Batch) * (size_t)(capstates / BATCH + 1));
    max_claims = ((size_t)1 << 27) >> shift;
    claims = shared(sizeof(Claim) * max_claims);
    results_size = ((size_t)1 << 34) >> shift;
    encodings_size = ((size_t)1 << 36) >> shift;
    results = xmalloc(sizeof(char *) * (size_t)nworkers);
    encodings = xmalloc(sizeof(char *) * (size_t)nworkers);
    for (int i = 0; i < nworkers; i++) {
        results[i] = shared(results_size);
        encodings[i] = shared(encodings_size);
    }
}

/* The claim of the state fp, made if `make` and there is none; NULL if
   there is none, or no room for it. */
static Claim *claim(uint64_t fp, int make, int *made) {
    uint64_t key = fp ? fp : 1;   /* 0 marks a free entry */
    size_t mask = control->nclaims - 1;
    for (size_t j = key & mask;; j = (j + 1) & mask) {
        uint64_t k = atomic_load(&claims[j].key);
        if (k == key) return &claims[j];
        if (k) continue;
        if (!make) return NULL;
        if (!w_budget) {
            /* Room for claims is taken a chunk at a time: the count is shared. */
            if (atomic_fetch_add(&control->claimed, CLAIM_CHUNK) + CLAIM_CHUNK > control->nclaims / 2) return NULL;
            w_budget = CLAIM_CHUNK;
        }
        w_budget--;
        if (atomic_compare_exchange_strong(&claims[j].key, &k, key)) { *made = 1; return &claims[j]; }
        if (k == key) return &claims[j];
    }
}

/* Checks the invariants of the successor a worker claimed and keeps its encoding. */
static void settle(Claim *c) {
    EvalState s = eval_save();
    sigjmp_buf here;
    handler = &here;
    volatile int ok = 0;
    if (!sigsetjmp(here, 0) && check_list(invariants, ninvariants, succ) < 0) {
        w_bytes.n = 0;
        for (int i = 0; i < nvars; i++) encode(&w_bytes, succ[i]);
        ok = w_encoded + w_bytes.n <= encodings_size;   /* else the search keeps it */
    }
    eval_restore(s);
    if (ok) {
        memcpy(encodings[w_id] + w_encoded, w_bytes.s, w_bytes.n);
        c->loc = (uint64_t)w_id << 40 | w_encoded;
        c->len = (int)w_bytes.n;
        w_encoded += w_bytes.n;
    }
    atomic_store(&c->state, ok ? SETTLED : FAILED);
}

static int worker_emit(void) {
    succ_count++;
    for (int i = 0; i < nvars; i++) if (!succ[i]) { w_clean = 0; return 1; }
    Found f = {0, cur_action, CONSTRAINED, 0};
    if (check_list(constraints, nconstraints, succ) < 0 &&
        check_actions(action_constraints, naction_constraints, cur_state, succ) < 0) {
        f.fp = fingerprint(succ);
        if (check_actions(action_props, naction_props, cur_state, succ) >= 0) { w_clean = 0; return 1; }
        f.seen = fp_find(f.fp);
        if (f.seen == NEW) {
            int made = 0;
            Claim *c = claim(f.fp, 1, &made);
            if (!c) { w_clean = 0; return 1; }
            if (made) settle(c);
            f.claim = (uint32_t)(c - claims);
        }
    } else if (check_list(invariants, ninvariants, succ) >= 0 || check_actions(action_props, naction_props, cur_state, succ) >= 0) {
        w_clean = 0;   /* outside the model: the search reports it */
        return 1;
    }
    if (w_n + sizeof f > results_room()) { w_clean = 0; return 1; }   /* the search expands it */
    memcpy(results[w_id] + w_n, &f, sizeof f);
    w_n += sizeof f;
    w_found++;
    return 0;
}

static void worker_expand(int idx) {
    size_t head = w_n;
    w_n += 2 * sizeof(int);
    w_clean = 1;
    w_found = 0;
    EvalState saved = eval_save();
    sigjmp_buf here;
    handler = &here;
    if (sigsetjmp(here, 0)) {
        w_clean = 0;
    } else {
        cur_level = states[idx].depth;
        cur_state = decode_state(idx);
        draw_restart(cur_state);
        succ_count = 0;
        for (int a = 0; a < nactions && w_clean; a++) {
            cur_action = a;
            memset(succ, 0, sizeof(Value *) * (size_t)nvars);
            next_state = succ;
            gen_target = succ;
            gen(actions[a].e, actions[a].c, NULL);
        }
        if (succ_count == 0 && check_deadlock) w_clean = 0;
    }
    eval_restore(saved);
    if (!w_clean) { w_n = head + 2 * sizeof(int); w_found = 0; }
    int header[2] = {w_clean, w_found};
    memcpy(results[w_id] + head, header, sizeof header);
    arena_reset(&scratch_arena);
}

/* Expands the next batch of the level, if any is left and there is room
   for its results; else gives it back to the search. */
static int expand_batch(void) {
    int b = atomic_fetch_add(&control->next, 1);
    if (b >= control->nbatches) return 0;
    if (w_n > results_room()) {
        batches[b].worker = -1;
        atomic_store(&batches[b].done, 1);
        return 0;
    }
    batches[b].worker = w_id;
    batches[b].off = w_n;
    int lo = control->lo + b * BATCH, hi = lo + BATCH < control->hi ? lo + BATCH : control->hi;
    for (int idx = lo; idx < hi; idx++) worker_expand(idx);
    atomic_store(&batches[b].done, 1);
    return 1;
}

static _Noreturn void worker_main(int id) {
    in_worker = 1;
    gen_emit = worker_emit;
    w_id = id;
    char c;
    while (read(to_worker[id][0], &c, 1) == 1) {
        fp_table = control->fp_table;
        fp_size = control->fp_size;
        w_n = w_encoded = w_budget = 0;
        while (expand_batch()) {}
        fflush(stdout);
        if (write(from_worker[id][1], "x", 1) != 1) _exit(1);
    }
    _exit(0);
}

static void start_workers(void) {
    fflush(stdout);
    to_worker = xmalloc(sizeof *to_worker * (size_t)nworkers);
    from_worker = xmalloc(sizeof *from_worker * (size_t)nworkers);
    worker_pids = xmalloc(sizeof(pid_t) * (size_t)nworkers);
    for (int i = 0; i < nworkers - 1; i++) {
        if (pipe(to_worker[i]) || pipe(from_worker[i])) { fputs("tpp: cannot start workers\n", stderr); exit(TPP_ERROR); }
        pid_t pid = fork();
        if (pid < 0) { fputs("tpp: cannot start workers\n", stderr); exit(TPP_ERROR); }
        if (pid == 0) {
            for (int j = 0; j < i; j++) { close(to_worker[j][1]); close(from_worker[j][0]); }
            close(to_worker[i][1]);
            close(from_worker[i][0]);
            worker_main(i);
        }
        close(to_worker[i][0]);
        close(from_worker[i][1]);
        worker_pids[i] = pid;
    }
    workers_running = 1;
    forked_strs = nstrs;
}

static void stop_workers(void) {
    if (!workers_running) return;
    for (int i = 0; i < nworkers - 1; i++) close(to_worker[i][1]);
    for (int i = 0; i < nworkers - 1; i++) waitpid(worker_pids[i], NULL, 0);
    workers_running = 0;
}

/* Whether the successors found for a state can merge: each new one was
   settled by the worker that claimed it, which may still be at it. */
static int mergeable(const Found *f, int nfound) {
    for (int k = 0; k < nfound; k++) {
        if (f[k].seen != NEW) continue;
        Claim *c = &claims[f[k].claim];
        int st;
        while ((st = atomic_load(&c->state)) == CLAIMED) sched_yield();
        if (st != SETTLED) return 0;
    }
    return 1;
}

static void merge_state(int idx, const Found *f, int nfound) {
    expanding = idx;
    expanded = idx + 1;
    for (int k = 0; k < nfound; k++) {
        generated++;
        if (f[k].seen == CONSTRAINED) continue;
        int found = f[k].seen;
        Claim *c = &claims[f[k].claim];
        /* New this level: merged already, or added by the search itself. */
        if (found == NEW && (found = c->merged - 1) < 0 && (found = fp_find(f[k].fp)) < 0) {
            size_t off = store.n;
            store_add(encodings[c->loc >> 40] + (c->loc & (((uint64_t)1 << 40) - 1)), (size_t)c->len);
            int depth = states[idx].depth + 1;
            found = add_record(off, f[k].fp, idx, f[k].action, depth);
            if (depth > max_depth) max_depth = depth;
        }
        if (f[k].seen == NEW) c->merged = found + 1;
        if (ntemporal_props) add_edge(idx, found, f[k].action);
    }
    note_expanded(idx);
}

static int merge_level(int lo, int hi);

static int expand_level(int lo, int hi) {
    /* Workers know the strings there were when they started; states may
       now hold newer ones. */
    if (workers_running && nstrs != forked_strs) stop_workers();
    if (!workers_running) start_workers();
    control->lo = lo;
    control->hi = hi;
    control->nbatches = (hi - lo + BATCH - 1) / BATCH;
    control->fp_table = fp_table;
    control->fp_size = fp_size;
    size_t want = (size_t)(4 * (hi - lo) * (growth > 1 ? growth : 1)), n = 4096;
    while (n < want && n < max_claims) n *= 2;
    memset(claims, 0, sizeof(Claim) * n);
    control->nclaims = n;
    atomic_store(&control->claimed, 0);
    atomic_store(&control->next, 0);
    for (int b = 0; b < control->nbatches; b++) atomic_store(&batches[b].done, 0);
    fflush(stdout);
    for (int i = 0; i < nworkers - 1; i++)
        if (write(to_worker[i][1], "x", 1) != 1) { fputs("tpp: a worker stopped\n", stderr); exit(TPP_ERROR); }
    w_id = nworkers - 1;
    w_n = w_encoded = w_budget = 0;
    int status = merge_level(lo, hi);
    atomic_store(&control->next, control->nbatches);   /* after an error, the workers stop */
    char c;
    for (int i = 0; i < nworkers - 1; i++)
        if (read(from_worker[i][0], &c, 1) != 1) { fputs("tpp: a worker stopped\n", stderr); exit(TPP_ERROR); }
    return status;
}

/* The search expands a batch as the workers do while it waits for one. */
static int steal_batch(void) {
    int (*saved_emit)(void) = gen_emit;
    in_worker = 1;
    gen_emit = worker_emit;
    int took = expand_batch();
    gen_emit = saved_emit;
    in_worker = 0;
    return took;
}

static int merge_level(int lo, int hi) {
    for (int b = 0; b < control->nbatches; b++) {
        while (!atomic_load(&batches[b].done))
            if (!steal_batch()) sched_yield();
        /* A batch given back has no results: the search expands it. */
        const char *p = batches[b].worker < 0 ? NULL : results[batches[b].worker] + batches[b].off;
        int end = lo + (b + 1) * BATCH < hi ? lo + (b + 1) * BATCH : hi;
        for (int idx = lo + b * BATCH; idx < end; idx++) {
            if (p) {
                int header[2];
                memcpy(header, p, sizeof header);
                const Found *f = (const Found *)(p + sizeof header);
                p += sizeof header + sizeof(Found) * (size_t)header[1];
                if (header[0] && mergeable(f, header[1])) { merge_state(idx, f, header[1]); continue; }
            }
            int status = expand_state(idx);
            if (status || exit_requested) return status;
        }
    }
    return 0;
}

/* The message of the error that stopped the search, then where it was. */
static void print_error_text(void) {
    printf("%s\n", error_message.s ? error_message.s : "");
    if (error_expr && error_expr->mod) {
        Buf b = {0};
        node_text(&b, error_expr);
        printf("%s\n", b.s);
        free(b.s);
    }
}

/* After an error, the function constructors that were being computed in
   full, innermost first, which TLC reports it cannot fingerprint (§11.3);
   else a blank line. */
static void print_unfingerprinted(Expr **fps, int n) {
    if (!n) { printf("\n"); return; }
    printf("Error: TLC was unable to fingerprint.\n\nFingerprint Stack Trace:\n");
    int m = 0;
    for (int i = 0; i < n; i++) if (!m || fps[m - 1] != fps[i]) fps[m++] = fps[i];
    for (int i = m - 1; i >= 0; i--) {
        Buf b = {0};
        loc_text(&b, fps[i]);
        printf("%d) %s\n", i, b.s);
        free(b.s);
    }
    printf("\nReason:\n");
    print_error_text();
}

/* Breadth-first search (§10.4). Returns the exit status. */
static int search(void) {
    if (dump_file) nworkers = 1;
    if (nworkers > 1) share_search();
    succ = NEWN(Value *, nvars);
    sigjmp_buf here;
    handler = &here;
    if (sigsetjmp(here, 0)) {
        handler = NULL;
        int status = eval_failure_status();
        int over = stackless;
        int nfp = fp_depth < 64 ? fp_depth : 64;
        Expr *fps[64];
        memcpy(fps, fp_trace, sizeof(Expr *) * (size_t)nfp);
        fp_depth = 0;
        if (expanding < 0) {
            if (!live_init_failed) print_error_message();
            if (error_state && error_module != 1 && !live_init_failed) {
                printf("While working on the initial state:\n");
                print_state(error_state);
                printf("\n");
            }
            /* After an error in a property TLC went on as it replays Init. */
            if (!live_init_failed) replay_error(NULL);
            if (over != 2) print_unfingerprinted(fps, nfp);
            print_counts(nstates);
            return TPP_FAILURE_EVAL;   /* TLC: a failed Assert too */
        }
        if (over == 2) {   /* TLC prints its Java stack */
            print_exception("java.lang.StackOverflowError", "java.lang.StackOverflowError\n\n");
            print_counts(nstates - expanded - checking_invariant());
            return checking_invariant() ? TPP_ERROR : TPP_FAILURE_EVAL;
        }
        int outside = fingerprinting;
        fingerprinting = 0;
        if (outside || checking(constraints, nconstraints) || checking(action_constraints, naction_constraints)) {
            /* TLC: an unexpected exception, without the behavior */
            print_exception("java.lang.RuntimeException", error_module == 1 ? ": tlc2.tool.EvalException: " : ": util.Assert$TLCRuntimeException: ");
            print_error_text();
            if (!over) print_unfingerprinted(fps, nfp);
            print_counts(outside ? nstates - expanded : 0);
            return error_module == 1 ? TPP_ERROR : TPP_FAILURE_EVAL;
        }
        if (checking(action_props, naction_props)) {
            printf("Error: Evaluating action property %s failed.\n", S(error_check->name));
            print_error_text();
            int failed = print_trace(expanding, error_state, cur_action);
            print_unfingerprinted(fps, nfp);
            print_counts(nstates - expanded - violation_fresh);
            return failed ? failed : error_module == 1 ? TPP_ERROR : TPP_FAILURE_EVAL;
        }
        if (checking_invariant()) {
            /* An invariant of a new state failed to evaluate: as TLC, an
               error rebuilding the behavior replaces it. */
            printf("Error: Evaluating invariant %s failed.\n", S(error_check->name));
            print_error_text();
            int failed = print_trace(expanding, error_state, cur_action);
            print_counts(nstates - expanded - 1);
            return failed ? failed : TPP_FAILURE_SAFETY_EVAL;
        }
        /* TLC reports an error of its own evaluator in Next as an unexpected
           exception; one of a standard module's operators plainly. */
        if (status == TPP_FAILURE_EVAL && error_module != 1)
            print_exception("java.lang.RuntimeException", ": ");
        else
            printf("Error: ");
        print_error_text();
        Value **from = decode_state(expanding);
        print_trace(expanding, NULL, 0);
        replay_error(from);
        print_unfingerprinted(fps, nfp);
        print_counts(nstates - expanded);
        if (status == TPP_FAILURE_EVAL && error_module == 1) status = TPP_ERROR;
        return status;
    }
    gen_target = succ;
    next_state = NULL;
    cur_state = succ;
    gen_init = 1;
    gen_emit = init_emit;
    if (ntemporal_props) live_init_lits();
    arena = &scratch_arena;
    draw_restart(NULL);
    gen(init_expr, root_ctx, NULL);
    arena_reset(&scratch_arena);
    if (live_init_failed) {   /* TLC stops, exiting as if no error */
        if (violation) printf("\n");
        print_counts(nstates);
        return TPP_OK;
    }
    if (violation) return violation;
    if (generated == nstates)
        printf("Finished computing initial states: %d distinct state%s generated.\n", nstates, nstates == 1 ? "" : "s");
    else
        printf("Finished computing initial states: %ld state%s generated, with %d of them distinct.\n", generated, generated == 1 ? "" : "s", nstates);
    fflush(stdout);
    if (!next_expr) {
        printf("Error: No next state actions defined to generate successor states from.\n");
        return TPP_FAILURE_EVAL;
    }
    gen_init = 0;
    gen_emit = next_emit;
    for (int lo = 0; lo < nstates;) {
        int hi = nstates, status = 0;
        if (nworkers > 1 && hi - lo >= PARALLEL_LEVEL) status = expand_level(lo, hi);
        else for (int idx = lo; idx < hi && !status && !exit_requested; idx++) status = expand_state(idx);
        if (status) return status;
        if (exit_requested) break;
        growth = (double)(nstates - hi) / (hi - lo);
        lo = hi;
    }
    expanding = -1;
    if (ntemporal_props) {
        arena = &perm_arena;
        collect_fairnesses();
        sigjmp_buf live;
        handler = &live;
        if (sigsetjmp(live, 0)) return live_error();
        if (live_violated()) {
            print_counts(exit_requested ? nstates - expanded : 0);
            return TPP_VIOLATION_LIVENESS;
        }
        handler = &here;
    }
    handler = NULL;
    printf("Model checking completed. No error has been found.\n");
    print_counts(exit_requested ? nstates - expanded : 0);
    return TPP_OK;
}

/* ---------------------------------------------------------------- simulation */

/* TLC's random generator of behaviors, counting its doubles (aril). */
static JRandom rng;
static int64_t rng_aril;

static void rng_seed(int64_t seed) {
    jr_seed(&rng, seed);
    rng_aril = 0;
}

/* An index below n, as TLC draws one: floor(nextDouble() * n). */
static int rng_index(int n) {
    rng_aril++;
    return (int)(jr_double(&rng) * n);   /* >= 0: truncation is floor */
}

/* One of the 1341 primes from 1277011 on, TLC's steps through the actions. */
static int rng_prime(void) {
    static int primes[1341];
    if (!primes[0])
        for (int p = 1277011, k = 0; k < 1341; p += 2) {
            int prime = 1;
            for (int d = 3; d * d <= p && prime; d += 2) prime = p % d != 0;
            if (prime) primes[k++] = p;
        }
    int i = 1341;
    while (i == 1341) i = rng_index(1341);
    return primes[i];
}

/* States in order, complete or not. A simulation keeps them as values, as
   TLC does: it fingerprints only what it must (a liveness check, a trace),
   so a state may hold a function TLC cannot fingerprint. */
typedef struct { Value ***a; int n, cap; } States;
static States sim_found;

static void sim_push(States *s, Value **st) {
    if (s->n == s->cap) {
        s->cap = s->cap ? 2 * s->cap : 64;
        s->a = xrealloc(s->a, sizeof(Value **) * (size_t)s->cap);
    }
    s->a[s->n++] = st;
}

static void materialize(Value *v);

/* A state found, normalized as TLC normalizes a state's values. */
static int sim_emit(void) {
    Value **st = snapshot(gen_target);
    for (int i = 0; i < nvars; i++)
        if (st[i]) materialize(st[i]);
    sim_push(&sim_found, st);
    return 0;
}

/* The successors of st by one action: TLC picks the first action at
   random, then steps through them by a random prime until one has. */
static int sim_next(Value **st) {
    sim_found.n = 0;
    int a = rng_index(nactions), p = rng_prime();
    for (int i = 0; i < nactions; i++) {
        cur_state = st;
        cur_action = a;
        memset(succ, 0, sizeof(Value *) * (size_t)nvars);
        next_state = succ;
        gen_target = succ;
        gen(actions[a].e, actions[a].c, NULL);
        if (sim_found.n) return 1;
        a = (a + p) % nactions;
    }
    return 0;
}

/* A random behavior up to its error, as TLC prints it: each state labeled
   with the first action that yields it, a step that changes nothing left
   out. TLC fingerprints every state first, which may fail: then it prints
   the exception instead, and so does sim_print, returning 0. */
static void sim_exception(void);

static int sim_print(States *trace) {
    printf("Error: The behavior up to this point is:\n");
    sigjmp_buf here, *outer = handler;
    handler = &here;
    if (sigsetjmp(here, 0)) {
        handler = outer;
        sim_exception();
        return 0;
    }
    uint64_t *fps = NEWN(uint64_t, trace->n);
    for (int i = 0; i < trace->n; i++) fps[i] = fingerprint(trace->a[i]);
    /* TLC finds the action of every step, a repeated state's too, and
       fails as the behavior did if finding it fails; a step it does not
       find again, as one drawn at random, ends in its NullPointerException. */
    for (int i = 0, k = 1; i < trace->n; i++) {
        int action = i ? replay(trace->a[i - 1], trace->a[i], i) : -1;
        if (i && !replay_found) {
            handler = outer;
            print_exception("java.lang.NullPointerException", ": ");
            printf("Cannot read field \"state\" because \"currentStateInfo\" is null\n");
            return 0;
        }
        if (!i || fps[i] != fps[i - 1]) print_numbered(k++, action, trace->a[i], i ? trace->a[i - 1] : NULL);
    }
    handler = outer;
    return 1;
}

/* TLC's simulator reports a TLCRuntimeException, or an error raised
   outside its evaluator, plainly, any other as unexpected. */
static void sim_exception(void) {
    if (error_module == 2 || stackless) {
        printf("Error: ");
        print_error_text();
        return;
    }
    print_exception(error_status == TPP_VIOLATION_ASSERT || error_module ? "tlc2.tool.EvalException" : "java.lang.RuntimeException", ": ");
    print_error_text();
}

/* A simulated behavior: its states and the actions into them. */
static int *sim_order, *sim_acts, sim_n;

/* Whether a temporal property fails on the behavior, as TLC checks one:
   each state may stutter or take its step. */
static int sim_violates(States *trace) {
    for (int i = 0; i < nstates; i++) free(states[i].edges);
    nstates = 0;
    store.n = 0;
    if (fp_size) memset(fp_table, 0, sizeof(FpSlot) * (size_t)fp_size);
    int prev = -1;
    sim_order = xrealloc(sim_order, sizeof(int) * (size_t)(trace->n + 1));
    sim_acts = xrealloc(sim_acts, sizeof(int) * (size_t)(trace->n + 1));
    sim_n = trace->n;
    for (int i = 0; i < trace->n; i++) {
        Value **st = trace->a[i];
        uint64_t fp = fingerprint(st);
        int s = fp_find(fp);
        if (s < 0) s = add_state(st, fp, prev, -1, i + 1);
        sim_acts[i] = prev < 0 || s == prev ? STUTTER : replay(trace->a[i - 1], st, i);
        if (prev >= 0) add_edge(prev, s, sim_acts[i]);
        sim_order[i] = s;
        prev = s;
    }
    int violated = live_violated();
    sim_n = 0;
    return violated;
}

static void sim_summary(int64_t seed, int64_t aril) {
    printf("The number of states generated: %ld\n", generated);
    printf("Simulation using seed %lld and aril %lld\n", (long long)seed, (long long)aril);
}

/* Behavior t, ended without an error, as TLC writes it for -simulate
   file=FILE: a module FILE_0_t (worker 0) that defines each state. */
static void sim_write(const char *file, long t, States *trace) {
    Buf name = {0};
    buf_printf(&name, "%s_0_%ld", file, t);
    FILE *f = fopen(name.s, "w");
    if (!f) fail("Cannot write the behavior to %s.", name.s);
    fprintf(f, "---------------- MODULE %s -----------------\n", name.s);
    for (int i = 0; i < trace->n; i++) {
        fprintf(f, "STATE_%d == \n", i + 1);
        fprint_state(f, trace->a[i]);
        fprintf(f, "\n\n");
    }
    fprintf(f, "=================================================\n");
    fclose(f);
    free(name.s);
}

/* Random simulation (§10.8): TLC's behaviors for the seed, from one worker.
   The initial states are kept for good; a behavior, with all it
   evaluates, until it ends. */
static int simulate(const TPPOptions *o) {
    int64_t seed = o->seed_given ? o->seed : (int64_t)time(NULL) * 1000003 ^ getpid();
    rng_seed(seed);
    for (int64_t i = 0; i < o->aril; i++) rng_index(1);
    int64_t aril = rng_aril;
    rng_seed(jr_long(&rng));
    long depth = o->depth ? o->depth : 100;
    nimplied_inits = 0;   /* TLC's simulator ignores the properties that are state predicates */
    succ = NEWN(Value *, nvars);
    sigjmp_buf here;
    handler = &here;
    static States init, trace;   /* static: read after an error jumps back */
    static Value **checked;      /* the successor being checked */
    static Value **current;      /* the state the behavior is in, as TLC's */
    static EvalState fresh;
    fresh = eval_save();
    if (sigsetjmp(here, 0)) {
        Arena *in_use = arena;
        eval_restore(fresh);     /* the error may have left an evaluation, primed */
        arena = in_use;
        handler = NULL;
        int invariant = checking_invariant();
        if (!trace.n && invariant) {
            printf("Error: ");
            print_error_text();
            printf("While working on the initial state:\n");
            print_state(error_state);
            printf("\n");
            sim_summary(seed, aril);
            return TPP_ERROR;
        }
        if (invariant || checking(action_props, naction_props)) {
            printf("Error: Evaluating %s %s failed.\n", invariant ? "invariant" : "action property", S(error_check->name));
            print_error_text();
            sim_push(&trace, checked);
            if (!sim_print(&trace)) return TPP_ERROR;
            sim_summary(seed, aril);
            return invariant ? TPP_FAILURE_SAFETY_EVAL : TPP_VIOLATION_LIVENESS;
        }
        sim_exception();
        int status = error_module == 2 ? error_status : TPP_ERROR;
        if (current && (!trace.n || trace.a[trace.n - 1] != current)) sim_push(&trace, current);
        if (trace.n) {
            error_check = NULL;
            sim_print(&trace);
        }
        return status;
    }
    static Arena init_arena;     /* the initial states, kept for good */
    arena = &init_arena;
    simulating = 1;
    gen_target = cur_state = NEWN(Value *, nvars);
    next_state = NULL;
    gen_init = 1;
    gen_emit = sim_emit;
    gen(init_expr, root_ctx, NULL);
    gen_init = 0;
    init = sim_found;
    sim_found = (States){0};
    generated = init.n;
    printf("Computed %d initial states...\n", init.n);
    for (int i = 0; i < init.n; i++) {
        Value **st = init.a[i];
        for (int v = 0; v < nvars; v++)
            if (!st[v]) {
                printf("Error: State is not completely specified by the initial predicate:\n");
                print_state(st);
                return TPP_ERROR;
            }
        int violated = check_initial(st);
        if (violated) return violated;
    }
    if (!init.n) {
        printf("Error: There is no state satisfying the initial state predicate.\n");
        return TPP_ERROR;
    }
    arena = &perm_arena;
    collect_fairnesses();
    arena = &scratch_arena;
    drawing = &draw_behaviors;   /* TLC's worker thread */
    int reported = 0;            /* -continue: a behavior ended in a violation */
    for (long t = 0; !o->traces || t < o->traces; t++) {
        arena_reset(&scratch_arena);
        trace.n = 0;
        Value **st = current = init.a[rng_index(init.n)];
        int in_model = check_list(constraints, nconstraints, st) < 0, ended = 0;
        for (long d = 0; d < depth && !ended; d++) {
            sim_push(&trace, st);
            cur_level = d + 1;
            if (!in_model) break;
            if (!sim_next(st)) {
                if (!check_deadlock) break;
                printf("Error: Deadlock reached.\n");
                if (!sim_print(&trace)) return TPP_ERROR;
                sim_summary(seed, aril);
                return TPP_VIOLATION_DEADLOCK;
            }
            for (int i = 0; i < sim_found.n; i++) {
                generated++;
                Value **s1 = checked = sim_found.a[i];
                for (int v = 0; v < nvars; v++)
                    if (!s1[v]) {
                        /* TLC prints the behavior, then fails to fingerprint the successor. */
                        printf("Error: Successor state is not completely specified by the next-state action.\n\n");
                        if (!sim_print(&trace)) return TPP_ERROR;
                        printf("\n");
                        print_exception("java.lang.NullPointerException", ": ");
                        printf("Cannot invoke \"tlc2.value.IValue.fingerPrint(long)\" because \"minVals[i]\" is null\n");
                        return TPP_ERROR;
                    }
                const char *what = "Invariant";
                Check *list = invariants;
                int status = TPP_VIOLATION_SAFETY;
                int bad = check_list(invariants, ninvariants, s1);
                if (bad < 0) what = "Action property", list = action_props, status = TPP_VIOLATION_LIVENESS, bad = check_actions(action_props, naction_props, st, s1);
                if (bad < 0) continue;
                printf("Error: %s %s is violated.\n", what, S(list[bad].name));
                sim_push(&trace, s1);
                if (!sim_print(&trace)) return TPP_ERROR;
                sim_summary(seed, aril);
                if (!keep_going) return status;
                reported = ended = 1;   /* as TLC, the behavior ends; the next begins */
                break;
            }
            if (ended) break;
            Value **s1 = sim_found.a[rng_index(sim_found.n)];
            in_model = check_list(constraints, nconstraints, s1) < 0 && check_actions(action_constraints, naction_constraints, st, s1) < 0;
            st = current = s1;
        }
        if (ended) continue;
        if (ntemporal_props && sim_violates(&trace)) {
            sim_summary(seed, aril);
            return TPP_VIOLATION_LIVENESS;
        }
        if (o->trace_file) sim_write(o->trace_file, t, &trace);
    }
    handler = NULL;
    printf("Progress: %ld states checked.\n", generated);
    return reported ? TPP_ERROR : TPP_OK;   /* TLC 2.19's exit after reporting */
}

/* ---------------------------------------------------------------- configuration */

/* A configuration statement (§9). */
typedef struct { const char *keyword; int name; int op; Value *value; int target, module; } CfgItem;   /* module: of name, in <- [M] */

static CfgItem *cfg;
static int ncfg;

static _Noreturn void config_error(const char *fmt, ...) {
    char msg[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    printf("Error: %s\n", msg);
    exit(TPP_ERROR_CONFIG);
}

/* line: where the lexer is; tline: the line of the token last read. */
typedef struct { const char *s; int pos, line, tline; } CfgLexer;

/* A configuration TLC cannot read: its ConfigFileException. */
static _Noreturn void cfg_fail(CfgLexer *L, const char *fmt, ...) {
    print_exception("tlc2.tool.ConfigFileException", ": ");
    printf("TLC found an error in the configuration file at line %d\n", L->tline);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    exit(TPP_ERROR);
}

static void cfg_space(CfgLexer *L) {
    for (;;) {
        const char *s = L->s + L->pos;
        if (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') { L->line += *s == '\n'; L->pos++; continue; }
        if (s[0] == '\\' && s[1] == '*') { while (L->s[L->pos] && L->s[L->pos] != '\n') L->pos++; continue; }
        if (s[0] == '(' && s[1] == '*') {
            int depth = 0;
            do {
                const char *t = L->s + L->pos;
                if (!*t) config_error("Unterminated comment in the configuration file.");
                if (t[0] == '(' && t[1] == '*') { depth++; L->pos += 2; }
                else if (t[0] == '*' && t[1] == ')') { depth--; L->pos += 2; }
                else { L->line += *t == '\n'; L->pos++; }
            } while (depth > 0);
            continue;
        }
        return;
    }
}

static const char *cfg_keywords[] = {
    "CONSTANT", "CONSTANTS", "INVARIANT", "INVARIANTS", "PROPERTY", "PROPERTIES", "CONSTRAINT",
    "CONSTRAINTS", "ACTION_CONSTRAINT", "ACTION_CONSTRAINTS",          /* lists */
    "INIT", "NEXT", "SPECIFICATION", "SYMMETRY", "VIEW", "CHECK_DEADLOCK", NULL};
enum { CFG_LISTS = 10 };

/* The next token: kind 'a' name, 'n' number (cfg_num), 's' string
   (cfg_str, interned unless cfg_scanning), 'p' punctuation, 0 end. Its
   text is cfg_text. */
static const char *cfg_text;
static int cfg_len, cfg_str, cfg_scanning;
static int64_t cfg_num;

static int cfg_token(CfgLexer *L) {
    cfg_space(L);
    const char *s = L->s + L->pos;
    if (!*s) return 0;
    L->tline = L->line;
    cfg_text = s;
    if (id_char(*s)) {
        int n = 0, digits = 1;
        while (id_char(s[n])) digits &= isdigit((unsigned char)s[n++]) != 0;
        L->pos += cfg_len = n;
        if (digits) { cfg_num = strtoll(s, NULL, 10); return 'n'; }
        return 'a';
    }
    if (*s == '"') {
        int n = 1;
        Buf b = {0};
        while (s[n] && s[n] != '"') {
            if (s[n] == '\\' && s[n + 1]) n++;
            buf_add(&b, s + n, 1);
            n++;
        }
        L->pos += cfg_len = n + 1;
        cfg_str = cfg_scanning ? -1 : intern_n(b.s ? b.s : "", (int)b.n);
        free(b.s);
        return 's';
    }
    L->pos += cfg_len = s[0] == '<' && s[1] == '-' ? 2 : 1;
    return 'p';
}

static int cfg_is(int kind, const char *text) {
    return kind && (int)strlen(text) == cfg_len && memcmp(cfg_text, text, (size_t)cfg_len) == 0;
}

/* The keyword the token is, if it is one. */
static int cfg_keyword(int kind) {
    for (int i = 0; kind == 'a' && cfg_keywords[i]; i++) if (cfg_is(kind, cfg_keywords[i])) return i;
    return -1;
}

/* The token as a name, interned unless cfg_scanning: TLC takes any token
   where a name is expected, and an operator symbol is one, joined from the
   characters that continue it: `++`, `\o`. */
static int cfg_name(CfgLexer *L, int kind) {
    if (kind == 'p') {
        int n = cfg_len;
        if (*cfg_text == '\\') while (isalpha((unsigned char)cfg_text[n])) n++;
        else while (cfg_text[n] && strchr("+-*/<>=!@#$%^&|~:.", cfg_text[n])) n++;
        L->pos += n - cfg_len;
        cfg_len = n;
    }
    return cfg_scanning ? -1 : intern_n(cfg_text, cfg_len);
}

/* A value: a number, a string, TRUE, FALSE, a set {v, ...}, or a model
   value, as TLC takes any other token. TLC makes the model values before
   it reads a module, so their names are interned even when cfg_scanning,
   when no value is made (§7.5). */
static Value *cfg_value(CfgLexer *L, int kind) {
    if (!kind) cfg_fail(L, "It was expecting a value, but did not find it.");
    if (cfg_is(kind, "{")) {
        List l = {0};
        kind = cfg_token(L);
        if (!cfg_is(kind, "}"))
            for (;;) {
                list_add(&l, cfg_value(L, kind));
                kind = cfg_token(L);
                if (cfg_is(kind, "}")) break;
                if (!cfg_is(kind, ",")) cfg_fail(L, "It was expecting }, but did not find it.");
                kind = cfg_token(L);
            }
        return cfg_scanning ? NULL : list_set(&l);
    }
    if (cfg_scanning && (kind == 'n' || kind == 's' || cfg_is(kind, "TRUE") || cfg_is(kind, "FALSE"))) return NULL;
    if (kind == 'n') return mk_int(cfg_num);
    if (kind == 's') return mk_str(cfg_str);
    if (cfg_is(kind, "TRUE")) return TRUEV;
    if (cfg_is(kind, "FALSE")) return FALSEV;
    int name = intern_n(cfg_text, cfg_len);
    return cfg_scanning ? NULL : mk_model(name);
}

static void cfg_add(const char *keyword, int name, int op, Value *v, int target, int module) {
    if (cfg_scanning) return;
    cfg = xrealloc(cfg, sizeof(CfgItem) * (size_t)(ncfg + 1));
    cfg[ncfg++] = (CfgItem){keyword, name, op, v, target, module};
}

/* Reads the configuration as TLC does. TLC reads it before any module, so
   a first pass, scanning, checks it and interns the model values' names;
   the second, after the modules, makes its statements. */
static void read_config(const char *text, int scanning) {
    CfgLexer L = {text, 0, 1, 1};
    int seen[16] = {0};
    cfg_scanning = scanning;
    int kind = cfg_token(&L);
    while (kind) {
        int key = cfg_keyword(kind);
        if (key < 0) cfg_fail(&L, "It was expecting a keyword, but did not find it.");
        const char *k = cfg_keywords[key];
        kind = cfg_token(&L);
        if (key < 2) {
            while (kind && cfg_keyword(kind) < 0) {
                int name = cfg_name(&L, kind), module = 0;
                kind = cfg_token(&L);
                if (cfg_is(kind, "(")) {   /* an operator constant's parameters: skipped */
                    while (kind && !cfg_is(kind, ")")) kind = cfg_token(&L);
                    if (!kind) cfg_fail(&L, "It was expecting a value, but did not find it.");
                    kind = cfg_token(&L);
                }
                int op = cfg_is(kind, "=") ? '=' : cfg_is(kind, "<-") ? '<' : 0;
                if (!op) cfg_fail(&L, "It was expecting = or <-, but did not find it.");
                kind = cfg_token(&L);
                if (cfg_is(kind, "[")) {   /* name = [M] value, name <- [M] target: name of module M */
                    module = cfg_name(&L, cfg_token(&L));
                    if (!cfg_is(cfg_token(&L), "]")) cfg_fail(&L, "It was expecting ], but did not find it.");
                    kind = cfg_token(&L);
                }
                if (op == '=') cfg_add(k, name, op, cfg_value(&L, kind), 0, module);
                else if (!kind) cfg_fail(&L, module ? "Expected an identifier after <-[mod]." : "Expected an identifier after <-.");
                else cfg_add(k, name, op, NULL, cfg_name(&L, kind), module);
                kind = cfg_token(&L);
            }
        } else if (key < CFG_LISTS) {
            for (; kind && cfg_keyword(kind) < 0; kind = cfg_token(&L)) cfg_add(k, cfg_name(&L, kind), 0, NULL, 0, 0);
        } else {
            if (!kind) cfg_fail(&L, "The keyword %s was not followed by an identifier.", k);
            if (seen[key]++) cfg_fail(&L, "The keyword %s appeared twice.", k);
            if (strcmp(k, "CHECK_DEADLOCK") == 0) {
                if (!cfg_is(kind, "TRUE") && !cfg_is(kind, "FALSE")) cfg_fail(&L, "It was expecting TRUE or FALSE, but did not find it.");
                cfg_add(k, 0, 0, cfg_is(kind, "TRUE") ? TRUEV : FALSEV, 0, 0);
            } else cfg_add(k, cfg_name(&L, kind), 0, NULL, 0, 0);
            kind = cfg_token(&L);
        }
    }
    cfg_scanning = 0;
}

/* ---------------------------------------------------------------- model setup */

static Sym *root_sym(int name) { return module_lookup(root, name); }

/* A call of the definition the configuration names, through the instances
   it is reached by. */
/* The definition the configuration names as what. TLC rejects one with
   parameters as a role (args 'r') or by name ('s'), or, for SYMMETRY (0),
   evaluates it. */
static Expr *config_call(int name, const char *what, int args) {
    Sym *s = root_sym(name);
    if (!s || (s->def->kind != D_OP && s->def->kind != D_FCN))
        config_error("The %s %s specified in the configuration file\nis not defined in the specification.", what, S(name));
    if (s->def->nparams && args == 'r') config_error("TLC requires %s not to take any argument, but one was given: %s", what, S(name));
    if (s->def->nparams && args == 's') config_error("TLC requires %s not to take any argument.", S(name));
    if (s->def->nparams) return s->def->body;   /* its parameters undefined there */
    Def *d = s->def;
    Expr *e = NEW(Expr);
    e->kind = K_CALL;
    e->def = d;
    e->mod = d->mod;
    if (d->where) { e->l1 = d->where->l1; e->c1 = d->where->c1; e->l2 = d->where->l2; e->c2 = d->where->c2; }
    if (s->npath) {
        e->kind = K_INSTCALL;
        e->defs = s->path;
        e->ndefs = e->nseg = s->npath;
        e->segs = NEWN(Seg, s->npath);
    }
    return e;
}

/* The body of a definition with no parameters, or the expression. */
static Expr *unfold(Expr *e) {
    for (int i = 0; i < 5000 && e->kind == K_CALL && e->n == 0; i++) {   /* a cycle by substitution is left to fail */
        Def *d = e->def->replacement ? e->def->replacement : e->def;   /* Init <- MCInit */
        if (d->kind != D_OP || d->native || d->nparams) break;
        e = d->body;
    }
    return e;
}

/* part, a subexpression of the body of the instance call's definition,
   as the instance sees it. */
static Expr *in_instance(Expr *call, Expr *part) {
    if (!call) return part;
    Def *d = NEW(Def);
    *d = *call->def;
    d->body = part;
    d->level = -1;
    d->cache = NULL;
    Expr *e = NEW(Expr);
    *e = *call;
    e->def = d;
    return e;
}

/* The value of a call of a constant definition; NULL if it is no constant
   or fails. TLC computes those but a literal's before reading the
   configuration (literal_def). */
static Value *constant_value(Expr *e) {
    every_argument = 1;
    int level = expr_level(e);
    every_argument = 0;
    if (level) return NULL;
    sigjmp_buf here;
    handler = &here;
    if (sigsetjmp(here, 0)) { handler = NULL; error_reset(); fp_depth = 0; return NULL; }
    Value *v = eval(e, root_ctx);
    handler = NULL;
    return v;
}

/* The conjuncts of a specification or property, each as the instance it
   is in sees it: a call of an instance's definition is followed into, as
   TLC does, unless the instance has parameters (IM(y)!Spec). In a
   specification, a constant definition named is its value: TRUE, which is
   dropped, or TLC's error. */
static void conjuncts(Expr *e, Expr *call, Expr ***out, int *n, int spec) {
    Value *v;
    if (spec && !call && e->kind == K_CALL && e->n == 0 && (v = constant_value(e))) {
        if (v->kind != V_BOOL) config_error("The operator %s, which equals %s,\ncannot be used as a spec", S(e->def->name), pretty(v));
        if (!v->u.i) config_error("The spec is trivially false because %s is false.", S(e->def->name));
        return;
    }
    e = unfold(e);
    if (e->kind == K_AND && eval_depth < 5000) {   /* as unfold */
        eval_depth++;
        for (int i = 0; i < e->n; i++) conjuncts(e->args[i], call, out, n, spec);
        eval_depth--;
        return;
    }
    if (!call && e->kind == K_INSTCALL && e->n == 0 && e->def->kind == D_OP && !e->def->native) {
        int plain = 1;
        for (int i = 0; i < e->ndefs; i++) plain &= e->defs[i]->nparams == 0;
        if (plain) { conjuncts(e->def->body, e, out, n, spec); return; }
    }
    *out = xrealloc(*out, sizeof(Expr *) * (size_t)(*n + 2));
    (*out)[(*n)++] = e;
    (*out)[(*n)++] = call;
}

static Expr *conjoin(Expr **parts, int n) {
    if (n == 1) return parts[0];
    Expr *e = NEW(Expr);
    *e = *parts[0];
    e->kind = K_AND;
    e->n = n;
    e->args = NEWN(Expr *, n);
    memcpy(e->args, parts, sizeof(Expr *) * (size_t)n);
    return e;
}

/* The variables in a subscript, as TLC collects them to warn of one
   missing: in tuples and the definitions they call, through instances;
   an operator's parameter stands for its argument, and an instance's
   variable for what it substitutes. */
typedef struct Args { Def *d; Expr **args; struct Args *up; } Args;
static void subscript_vars(Expr *e, Args *a, Def **path, int npath, char *in, int depth) {
    if (depth > 100) return;
    switch (e->kind) {
    case K_DECL:
        if (e->def->var_index >= 0 && e->def->kind == D_VAR) in[e->def->var_index] = 1;
        for (int k = npath - 1; k >= 0; k--)
            for (int j = 0; j < path[k]->nsubst; j++)
                if (path[k]->subst_decl[j] == e->def) { subscript_vars(path[k]->subst_expr[j], NULL, path, k, in, depth + 1); return; }
        return;
    case X_TUPLE:
        for (int i = 0; i < e->n; i++) subscript_vars(e->args[i], a, path, npath, in, depth + 1);
        return;
    case X_LET:
        subscript_vars(e->args[0], a, path, npath, in, depth + 1);
        return;
    case K_BOUND:
        for (; a; a = a->up)
            for (int i = 0; i < a->d->nparams; i++)
                if (a->d->params[i] == e->def) { subscript_vars(a->args[i], a->up, path, npath, in, depth + 1); return; }
        return;
    case K_CALL: {
        Def *d = e->def->replacement ? e->def->replacement : e->def;
        if (d->kind != D_OP || d->native || d->nparams != e->n) return;
        Args b = {d, e->args, a};
        subscript_vars(d->body, &b, path, npath, in, depth + 1);
        return;
    }
    case K_INSTCALL:
        if (e->def->kind == D_OP && !e->def->native && !e->n && !npath)
            subscript_vars(e->def->body, NULL, e->defs, e->ndefs, in, depth + 1);
        return;
    default: return;
    }
}

/* SPECIFICATION Init /\ [][Next]_v /\ fairness (§9.1). */
static void use_specification(Expr *spec) {
    Expr **parts = NULL, **init = NULL;
    int n = 0, ninit = 0, nnext = 0;
    conjuncts(spec->def->body, spec->kind == K_INSTCALL ? spec : NULL, &parts, &n, 1);
    for (int i = 0; i < n; i += 2) {
        Expr *e = parts[i], *call = parts[i + 1];
        if (expr_level(e) <= 1) {
            init = xrealloc(init, sizeof(Expr *) * (size_t)(ninit + 1));
            init[ninit++] = in_instance(call, e);
        } else if (e->kind == K_ALWAYS && unfold(e->args[0])->kind == X_SQACT) {
            if (nnext++)
                config_error("The specification contains more than one conjunct of the form [][Next]_v,\n"
                             "but TLC can handle only specifications with one next-state relation.");
            Expr *sq = unfold(e->args[0]);
            next_expr = in_instance(call, sq->args[0]);
            spec_subscript = in_instance(call, sq->args[1]);
            if (!nowarning) {
                char *in = calloc((size_t)nvars + 1, 1);
                subscript_vars(spec_subscript, NULL, NULL, 0, in, 0);
                for (int v = 0; v < nvars; v++)
                    if (!in[v]) {
                        char text[256];
                        snprintf(text, sizeof text, "The subscript of the next-state relation specified by the specification\n"
                                 "does not seem to contain the state variable %s", S(vars[v]->name));
                        warning(text);
                    }
                free(in);
            }
        } else if (expr_level(e) == 2) {
            Buf b = {0};
            loc_text(&b, e);
            config_error("TLC cannot handle this conjunct of the spec:\n%s", b.s);
        } else {
            fairness = xrealloc(fairness, sizeof(Expr *) * (size_t)(nfairness + 1));
            fairness[nfairness++] = in_instance(call, e);
        }
    }
    if (ninit) init_expr = conjoin(init, ninit);
    free(parts);
    free(init);
}

/* The parts of a property, named as TLC names them: a conjunct by its
   location, unless it calls a definition, or is []P of one, named by
   that definition. */
typedef struct { Expr *e, *call; int name; } Part;
static void property_parts(Expr *e, Expr *call, int name, Part **out, int *n) {
    if (e->kind == K_CALL && e->n == 0 && eval_depth < 5000) {
        Def *d = e->def->replacement ? e->def->replacement : e->def;
        if (d->kind == D_OP && !d->native && !d->nparams) {
            eval_depth++;
            property_parts(d->body, call, e->def->name, out, n);
            eval_depth--;
            return;
        }
    }
    if (e->kind == K_AND) {
        for (int i = 0; i < e->n; i++) {
            Buf b = {0};
            loc_text(&b, e->args[i]);
            property_parts(e->args[i], call, intern(b.s), out, n);
            free(b.s);
        }
        return;
    }
    if (!call && e->kind == K_INSTCALL && e->n == 0 && e->def->kind == D_OP && !e->def->native) {
        int plain = 1;
        for (int i = 0; i < e->ndefs; i++) plain &= e->defs[i]->nparams == 0;
        if (plain) { property_parts(e->def->body, e, name, out, n); return; }
    }
    if (e->kind == K_ALWAYS && e->args[0]->kind == K_CALL && e->args[0]->n == 0) name = e->args[0]->def->name;
    *out = xrealloc(*out, sizeof(Part) * (size_t)(*n + 1));
    (*out)[(*n)++] = (Part){e, call, name};
}

/* A property: []P as an invariant, [][A]_v as an action property, the
   rest checked on the state graph (§10.5). */
static void use_property(Expr *p) {
    Def *d = p->def;
    Part *parts = NULL;
    int n = 0;
    property_parts(p->kind == K_INSTCALL ? d->body : p, p->kind == K_INSTCALL ? p : NULL, d->name, &parts, &n);
    for (int i = 0; i < n; i++) {
        Expr *e = parts[i].e, *call = parts[i].call;
        int name = parts[i].name;
        if (expr_level(e) <= 1) { add_check(&implied_inits, &nimplied_inits, d, in_instance(call, e), name); continue; }
        if (expr_level(e) == 2) config_error("The property %s is not correctly defined.", S(name));   /* an action, not under [] */
        if (e->kind == K_ALWAYS) {
            Expr *body = unfold(e->args[0]);
            if (expr_level(body) <= 1) { add_check(&invariants, &ninvariants, d, in_instance(call, body), name); continue; }
            if (body->kind == X_SQACT) { add_check(&action_props, &naction_props, d, in_instance(call, body), name); continue; }
        }
        add_check(&temporal_props, &ntemporal_props, d, in_instance(call, e), name);
    }
    free(parts);
}

static Value *def_body(Def *d) { return d->kind == D_FCN ? fcndef_value(d, (Ctx){NULL, NULL}) : eval(d->body, (Ctx){NULL, NULL}); }

/* The value of a constant definition, normalized, which TLC computes once.
   What computing it made is dropped once the value is copied out; a value
   holding its context, as a lazy function does, is computed again where it
   is kept, with the same draws, printing nothing again. After an error it
   is kept: it may be named. */
static Value *def_value(Def *d) {
    Arena *saved = arena, made = {0};
    JRandom draw = *drawing;
    int due = draw_due, loops = nregions, quiet = reprinting;
    sigjmp_buf *outer = handler, here;
    handler = &here;
    if (sigsetjmp(here, 0)) { handler = outer; arena = saved; nregions = loops; reprinting = quiet; raise_error(); }
    arena = &made;
    Value *v = def_body(d);
    materialize(v);
    arena = &perm_arena;
    long budget = (long)(arena_bytes(&made) / sizeof(Value));
    Value *c = copy_value(v, &budget);
    arena_free(&made);
    if (!c) {
        *drawing = draw;
        draw_due = due;
        reprinting = 1;
        materialize(c = def_body(d));
        reprinting = quiet;
    }
    handler = outer;
    arena = saved;
    return c;
}

/* Constant definition d evaluated once, any error ignored. */
static void evaluate_constant(Def *d) {
    EvalState s = eval_save();
    sigjmp_buf here;
    handler = &here;
    if (d->kind == D_FCN && !d->self && d->local != 2 && d->level == 0 && !sigsetjmp(here, 0)) {
        d->cache = def_value(d);   /* TLC keeps a function definition computed in full, if it can be */
    }
    if (d->kind == D_OP && !d->nparams && !d->native && !d->cache && def_level(d) == 0) {
        if (!sigsetjmp(here, 0)) call_def(d, NULL, 0, (Ctx){NULL, NULL}, NULL);
        else d->uncached = 1;
    }
    eval_restore(s);
}

/* TLC evaluates the constant definitions of each module at startup, in
   the order SANY finishes the modules (a module after those it uses): a
   constant replaced by a definition must evaluate as a constant, and every
   other constant definition without parameters is evaluated once, any
   error ignored. Side effects such as Print show then. */
static void evaluate_constants(Module *m) {
    if (m->instantiated && m->ndecls) return;
    Arena *saved = arena;
    arena = &perm_arena;
    for (int i = 0; i < m->ndecls; i++) {
        Def *c = m->decls[i];
        if (c->kind != D_CONST || !c->replacement || c->replacement->nparams) continue;
        sigjmp_buf here;
        handler = &here;
        if (sigsetjmp(here, 0)) {
            handler = NULL;
            Buf b = {0};
            node_text(&b, error_expr);
            config_error("The configuration file substitutes constant %s with non-constant %s%s%s%s%s", S(c->name),
                         S(c->replacement->name), error_module == 1 ? "" : " - specifically: ",
                         error_module == 1 ? "" : error_message.s, error_expr ? "\n" : "", error_expr ? b.s : "");
        }
        call_def(c->replacement, NULL, 0, (Ctx){NULL, NULL}, NULL);
        handler = NULL;
    }
    for (int i = 0; i < m->nunits; i++) if (m->units[i].kind == 0) evaluate_constant(m->units[i].def);
    arena = saved;
    for (int i = 0; i < m->nnested; i++) evaluate_constants(m->nested[i]);
}

/* TLC's status for assumption u: 0 if it holds. */
static int check_assumption(Unit *u) {
    Arena *saved = arena, made = {0};   /* dropped after, as the value is a Boolean's */
    sigjmp_buf here;
    handler = &here;
    if (sigsetjmp(here, 0)) {
        handler = NULL;
        arena = saved;
        Buf b = {0};
        loc_text(&b, u->expr);
        printf("Error: Evaluating assumption %s failed.\n%s", b.s, error_message.s ? error_message.s : "");
        if (error_expr && error_expr->mod) {
            Buf l = {0};
            node_text(&l, error_expr);
            printf("\n%s", l.s);
        }
        printf("\n");
        return TPP_VIOLATION_ASSUMPTION;
    }
    arena = &made;
    Value *v = eval(u->expr, root_ctx);
    arena = saved;
    handler = NULL;
    int holds = v->kind == V_BOOL && v->u.i;
    arena_free(&made);
    if (!holds) {
        Buf b = {0};
        loc_text(&b, u->expr);
        printf("Error: Assumption %s is false.\n", b.s);
        return TPP_VIOLATION_ASSUMPTION;
    }
    return 0;
}

static void check_assumptions(Module *m, int *done_status) {
    for (int i = 0; i < m->nunits; i++) {
        int status = m->units[i].kind == 2 ? check_assumption(&m->units[i]) : 0;
        if (status) { *done_status = status; return; }
    }
}

/* ---------------------------------------------------------------- TLC's order of the variables */

/* TLC lists the variables of a state in the order the root module's
   context, a java.util.Hashtable, enumerates them, reversed. That order
   follows from the names the context holds and the order SANY put them
   in: its built-in operators, in reverse, then the module's symbols as
   SANY meets them. */
typedef struct { const char *name; int module; int local; Def *def; } CtxName;
typedef struct { CtxName *a; int n, cap; } CtxNames;

static const char *sany_builtins[] = {
    "STRING", "FALSE", "TRUE", "BOOLEAN", "=", "/=", ".", "'", "\\lnot", "\\neg", "\\land",
    "\\lor", "\\equiv", "=>", "SUBSET", "UNION", "DOMAIN", "\\subseteq", "\\in", "\\notin", "\\",
    "\\intersect", "\\union", "\\times", "~>", "[]", "<>", "ENABLED", "UNCHANGED", "\\cdot",
    "-+->", "$AngleAct", "$BoundedChoose", "$BoundedExists", "$BoundedForall", "$CartesianProd",
    "$Case", "$ConjList", "$DisjList", "$Except", "$FcnApply", "$FcnConstructor", "$IfThenElse",
    "$NonRecursiveFcnSpec", "$Pair", "$RcdConstructor", "$RcdSelect", "$RecursiveFcnSpec", "$Seq",
    "$SetEnumerate", "$SetOfAll", "$SetOfFcns", "$SetOfRcds", "$SF", "$SquareAct", "$SubsetOf",
    "$TemporalExists", "$TemporalForall", "$TemporalWhile", "$Tuple", "$UnboundedChoose",
    "$UnboundedExists", "$UnboundedForall", "$WF", "$Nop", "$Qed", "$Pfcase", "$Have", "$Take",
    "$Pick", "$Witness", "$Suffices", NULL
};

static void ctx_put(CtxNames *c, CtxName e) {
    for (int i = 0; i < c->n; i++) if (c->a[i].module == e.module && strcmp(c->a[i].name, e.name) == 0) return;
    if (c->n == c->cap) {
        c->cap = c->cap ? c->cap * 2 : 64;
        c->a = xrealloc(c->a, sizeof(CtxName) * (size_t)c->cap);
    }
    c->a[c->n++] = e;
}

static CtxNames context_enumeration(Module *m);

/* The symbols of m's context, in the order SANY adds them. */
static CtxNames context_names(Module *m) {
    CtxNames c = {0};
    for (int i = 0; i < m->nextends; i++) {
        CtxNames x = context_names(load_module(m->extends[i], m));
        for (int k = 0; k < x.n; k++) if (!x.a[k].local) ctx_put(&c, x.a[k]);
        free(x.a);
    }
    for (int i = 0; i < m->nunits; i++) {
        Unit *u = &m->units[i];
        Def *d = u->def;
        if (u->kind == 2) continue;
        if (u->kind == 3) { ctx_put(&c, (CtxName){S(u->module->name), 1, 0, NULL}); continue; }
        if (u->kind == 1 || d->kind == D_INST) {
            /* An instance brings its module's definitions, then its theorems
               and assumptions, as that module's context enumerates them;
               a named instance I brings Op as I!Op, and instances too. */
            CtxNames t = context_enumeration(d->target);
            for (int pass = 0; pass < 2; pass++)
                for (int k = 0; k < t.n; k++) {
                    CtxName e = t.a[k];
                    if (e.local || e.module) continue;
                    int kind = e.def->kind;
                    int op = kind == D_OP || kind == D_FCN || kind == D_RECURSIVE || (kind == D_INST && u->kind == 0);
                    if (pass == 0 ? !op : kind != D_THM) continue;
                    if (u->kind == 0) {
                        char *name = xmalloc(strlen(S(d->name)) + strlen(e.name) + 2);
                        sprintf(name, "%s!%s", S(d->name), e.name);
                        e.name = name;
                    }
                    e.local = u->local;
                    ctx_put(&c, e);
                }
            free(t.a);
            if (u->kind == 1) continue;
        }
        ctx_put(&c, (CtxName){S(d->name), 0, u->local, d});
    }
    return c;
}

static unsigned java_hash(const char *s) {
    unsigned h = 0;
    for (; *s; s++) h = 31 * h + (unsigned char)*s;
    return h & 0x7fffffff;
}

/* The symbols of m's context, in the order its Hashtable enumerates them:
   the table holds SANY's built-in operators, put in reverse, then the
   symbols. It starts with 11 buckets and grows to 2n + 1 when it is three
   quarters full, putting each bucket's entries, from the last bucket to
   the first, at the head of their new bucket. */
static CtxNames context_enumeration(Module *m) {
    CtxNames c = context_names(m);
    int nbuiltins = (int)(sizeof sany_builtins / sizeof *sany_builtins) - 1, nkeys = nbuiltins + c.n;
    unsigned *hash = xmalloc(sizeof(unsigned) * (size_t)nkeys);
    int *link = xmalloc(sizeof(int) * (size_t)nkeys);
    for (int k = 0; k < nkeys; k++) hash[k] = java_hash(k < nbuiltins ? sany_builtins[nbuiltins - 1 - k] : c.a[k - nbuiltins].name);
    int cap = 11, count = 0, *head = xmalloc(sizeof(int) * (size_t)cap);
    for (int i = 0; i < cap; i++) head[i] = -1;
    for (int k = 0; k < nkeys; k++) {
        if (count >= cap * 3 / 4) {
            int bigger = cap * 2 + 1, *heads = xmalloc(sizeof(int) * (size_t)bigger);
            for (int i = 0; i < bigger; i++) heads[i] = -1;
            for (int i = cap - 1; i >= 0; i--)
                for (int e = head[i], next; e >= 0; e = next) {
                    next = link[e];
                    link[e] = heads[hash[e] % (unsigned)bigger];
                    heads[hash[e] % (unsigned)bigger] = e;
                }
            free(head);
            cap = bigger;
            head = heads;
        }
        link[k] = head[hash[k] % (unsigned)cap];
        head[hash[k] % (unsigned)cap] = k;
        count++;
    }
    CtxNames out = {0};
    for (int i = cap - 1; i >= 0; i--)
        for (int e = head[i]; e >= 0; e = link[e])
            if (e >= nbuiltins) ctx_put(&out, c.a[e - nbuiltins]);
    free(hash);
    free(link);
    free(head);
    free(c.a);
    return out;
}

/* Puts the variables in TLC's order: its context's, reversed. */
static void order_variables(void) {
    CtxNames c = context_enumeration(root);
    Def **order = NEWN(Def *, nvars);
    int n = nvars;
    for (int i = 0; i < c.n; i++)
        if (c.a[i].def && c.a[i].def->kind == D_VAR && n > 0) order[--n] = c.a[i].def;
    if (n == 0 && nvars) memcpy(vars, order, sizeof(Def *) * (size_t)nvars);
    for (int i = 0; i < nvars; i++) vars[i]->var_index = i;
    free(c.a);
}

/* The text of e as SANY's human-readable image: its tokens, without the
   spaces between them. */
static void expr_image(Buf *b, Expr *e) {
    for (int i = 0; i < e->mod->ntoks; i++) {
        Tok *t = &e->mod->toks[i];
        if (t->line < e->l1 || (t->line == e->l1 && t->col < e->c1)) continue;
        if (t->line > e->l2 || (t->line == e->l2 && t->col > e->c2)) break;
        if (t->kind == T_STR) buf_printf(b, "\"%.*s\"", t->len, t->text);
        else buf_puts(b, S(t->id));
    }
}

/* Whether a permutation of g moves the model value m. */
static int symmetry_moves(Value *g, Value *m) {
    for (int i = 0; i < g->n; i++) {
        Value *f = realize(g->u.set.elems[i]);
        for (int j = 0; j < f->n; j++)
            if (veq(f->u.fcn.keys[j], m)) { if (!veq(f->u.fcn.vals[j], m)) return 1; break; }
    }
    return 0;
}

/* TLC's warning for the sets of SYMMETRY body, with value g, that permute
   nothing, found as TLC's Tool.getSymmetryPerms finds them: among the
   arguments of the body, all of them when there are as many as functions
   in g, else each Permutations(C) none of whose model values g moves. */
static void warn_symmetry_sets(Expr *body, Value *g) {
    Buf names = {0};
    int n = 0;
    for (int i = 0; i < body->n; i++) {
        Expr *a = body->args[i];
        if (body->n < g->n) {
            if (a->kind != K_CALL || a->n != 1 || a->def->name != intern("Permutations")) continue;
            Expr *c = a->args[0];
            if (!(c->kind == K_DECL && c->def->kind == D_CONST) && !(c->kind == K_CALL && !c->n && !def_level(c->def))) continue;
            Value *set = eval(c, root_ctx);
            if (set->kind != V_SET) continue;
            int moves = 0;
            for (int j = 0; j < set->n && !moves; j++)
                moves = set->u.set.elems[j]->kind == V_MODEL && symmetry_moves(g, set->u.set.elems[j]);
            if (moves) continue;
        }
        Buf image = {0};
        expr_image(&image, a);
        char *name = image.s ? image.s : "";
        size_t len = strlen(name);
        if (strncmp(name, "Permutations", 12) == 0 && len > 13) { name[len - 1] = 0; name += 13; }
        int id = intern(name);
        for (int j = 0; j < ncfg; j++)
            if (strncmp(cfg[j].keyword, "CONSTANT", 8) == 0 && cfg[j].op != '=' && !cfg[j].module && cfg[j].target == id) { name = (char *)S(cfg[j].name); break; }
        buf_printf(&names, "%s%s", n++ ? ", and " : "", name);
        free(image.s);
    }
    if (n) {
        Buf b = {0};
        buf_printf(&b, "The set%s %s %s been defined to be a symmetry set but contain%s less than two elements.",
                   n > 1 ? "s" : "", names.s, n > 1 ? "have" : "has", n > 1 ? "" : "s");
        warning(b.s);
        free(b.s);
    }
    free(names.s);
}

/* Whether to check the invariant or property e the configuration names.
   As TLC, a constant definition is evaluated as the configuration is read,
   unless it is a literal or fails: TRUE is dropped, another value is an
   error. */
static int literal_def(Expr *e) { return e->def->body->kind == X_NUM || e->def->body->kind == X_STR; }

static int config_check(Expr *e, const char *what) {
    Value *v = literal_def(e) ? NULL : constant_value(e);
    if (!v) return 1;
    if (v->kind == V_BOOL && v->u.i) {
        if (!strcmp(what, "invariant")) {
            Buf b = {0};
            buf_printf(&b, "The invariant %s is a constant-level formula (i.e., it contains no variables, primes, or temporal operators) and evaluates to TRUE. "
                       "To assert constant-level formulas in your spec, use ASSUME ConstInv. If you optionally want to give the assumption a name, write ASSUME YourAssumption == ConstInv instead. "
                       "See https://explain.tlapl.us/assumptions-and-invariants for additional details.", S(e->def->name));
            warning(b.s);
            free(b.s);
        }
        return 0;
    }
    config_error("The %s of %s is equal to %s", what, S(e->def->name), pretty(v));
}

/* The root module's constants in the order TLC takes them: SANY keeps the
   module's symbols, with its 72 built-in operators, in a Java Hashtable,
   and TLC reverses its enumeration, so a constant comes in the order of
   its name's bucket. */
static int java_bucket(int name, int cap) {
    uint32_t h = 0;
    for (const char *p = S(name); *p; p++) h = 31 * h + (unsigned char)*p;
    return (int)((h & 0x7fffffff) % (uint32_t)cap);
}

static Def **tlc_constants(int *n) {
    int cap = 11, count = 72 + root->nsyms;
    for (int k = 0; k < count; k++) if (k >= (int)(cap * 0.75)) cap = cap * 2 + 1;
    Def **c = NEWN(Def *, root->ndecls);
    *n = 0;
    for (int i = 0; i < root->ndecls; i++) {
        Def *d = root->decls[i];
        if (d->kind != D_CONST) continue;
        int j = (*n)++;
        while (j > 0 && java_bucket(c[j - 1]->name, cap) > java_bucket(d->name, cap)) { c[j] = c[j - 1]; j--; }
        c[j] = d;
    }
    return c;
}

/* Whether the configuration substitutes for name, in module (0: the root). */
static int substituted(int name, int module) {
    for (int i = 0; i < ncfg; i++)
        if (cfg[i].op == '<' && cfg[i].name == name && cfg[i].module == module) return 1;
    return 0;
}

/* TLC's checks of a substitution for d, configured for module (0: the root). */
static void check_substitution(Def *d, int module) {
    if (!d->replacement && !d->bad_target) return;
    int target = d->bad_target ? d->bad_target : d->replacement->name;
    if (substituted(target, module))
        config_error("In the configuration file, the identifier %s appears\non the right-hand side of a <- after already appearing on the\nleft-hand side of one.", S(target));
    if (d->bad_target)
        config_error("The configuration file substitutes for %s with the undefined identifier %s.", S(d->name), S(d->bad_target));
    if (d->kind != D_CONST && d->replacement->kind != D_CONST && d->nparams != d->replacement->nparams)
        config_error("The configuration file substitutes for %s with %s of different number of arguments.", S(d->name), S(target));
}

static int setup_model(void) {
    for (int i = 0; i < root->ndecls; i++)
        if (root->decls[i]->kind == D_VAR) {
            vars = xrealloc(vars, sizeof(Def *) * (size_t)(nvars + 1));
            vars[nvars++] = root->decls[i];
        }
    order_variables();
    Expr *spec = NULL, *init = NULL, *next = NULL, *e;
    int stray = 0, module_stray = 0, no_module = 0;   /* a name substituted for, or a module, the spec does not have */
    /* TLC takes the constants, then the invariants, then the rest but the
       constraints, then the constraints, then the action constraints. */
    for (int pass = 0; pass < 5; pass++) {
    if (pass == 1) {
        int n;
        Def **c = tlc_constants(&n);
        for (int i = 0; i < n; i++)
            if (!c[i]->value && !c[i]->replacement && !c[i]->bad_target)
                config_error("The constant parameter %s is not assigned a value by the configuration file.", S(c[i]->name));
        for (int i = 0; i < n; i++) check_substitution(c[i], 0);
        for (int i = 0; i < root->nsyms; i++) check_substitution(root->syms[i].def, 0);
        if (stray) config_error("In the configuration file, the identifier %s does not appear in the specification.", S(stray));
        for (int i = 0; i < ncfg; i++) {
            Module *m = cfg[i].module ? find_module(cfg[i].module) : NULL;
            Sym *s = m ? module_lookup(m, cfg[i].name) : NULL;
            if (s) check_substitution(s->def, cfg[i].module);
        }
        if (module_stray) config_error("In the configuration file, the identifier %s does not appear in the specification.", S(module_stray));
        if (no_module) config_error("In the configuration file, the module name %s is not a module in the specification.", S(no_module));
    }
    for (int i = 0; i < ncfg; i++) {
        CfgItem *it = &cfg[i];
        const char *k = it->keyword;
        if (pass != (strncmp(k, "CONSTANT", 8) == 0 ? 0 : strncmp(k, "INVARIANT", 9) == 0 ? 1 : strncmp(k, "CONSTRAINT", 10) == 0 ? 3
                     : strncmp(k, "ACTION_CONSTRAINT", 17) == 0 ? 4 : 2)) continue;
        if (pass == 0) {
            Module *m = it->module ? find_module(it->module) : root;
            Sym *s = m ? module_lookup(m, it->name) : NULL;
            int *missing = !m ? &no_module : it->module ? &module_stray : &stray;
            if (!s && it->op != '=' && !*missing) *missing = m ? it->name : it->module;
            if (!s) continue;
            Def *d = s->def;
            if (it->op == '=') d->value = it->value;
            else {
                Sym *t = root_sym(it->target);
                if (t) d->replacement = t->def;
                else d->bad_target = it->target;   /* reported with the constants, as TLC */
            }
        } else if (strcmp(k, "INIT") == 0 || strcmp(k, "NEXT") == 0) {
            const char *what = k[0] == 'I' ? "initial predicate" : "next state action";
            e = config_call(it->name, what, 'r');
            if (!literal_def(e) && constant_value(e)) config_error("The %s %s cannot be a constant.", what, S(it->name));
            *(k[0] == 'I' ? &init : &next) = e;
        } else if (strcmp(k, "SPECIFICATION") == 0) spec = config_call(it->name, "name", 's');
        else if (strncmp(k, "INVARIANT", 9) == 0) {
            e = config_call(it->name, "invariant", 'r');
            if (expr_level(e) >= 2) {
                printf("Error: The invariant %s is not a state predicate (one with no primes or temporal operators).\n", S(it->name));
                if (level_params(e))
                    printf("Note that a bug can cause TLC to incorrectly report this error.\n"
                           "If you believe your TLA+ or PlusCal specification to be correct,\n"
                           "please check if this bug described in LevelNode.java starting at line 590ff affects you.\n");
                exit(TPP_FAILURE_SAFETY_EVAL);
            }
            if (config_check(e, "invariant")) add_check(&invariants, &ninvariants, e->def, e, e->def->name);
        } else if (strncmp(k, "PROPERT", 7) == 0) {
            e = config_call(it->name, "property", 's');
            if (expr_level(e) == 1) {
                Buf b = {0};
                const char *p = S(it->name);
                buf_printf(&b, "The formula %s is a state-level formula, but it was used as a PROPERTY (or PROPERTIES), "
                           "where a temporal formula is typically expected. State-level formulas used as PROPERTY "
                           "or PROPERTIES are only checked in the initial state. To verify that the formula %s "
                           "holds in all states of every behavior, use INVARIANT %s instead. Alternatively, "
                           "applying the \"always\" temporal operator ([]) to the state-level formula %s changes it "
                           "into a temporal formula, asserting that %s holds in all states of every behavior. "
                           "See https://explain.tlapl.us/invariants-and-properties for additional details.", p, p, p, p, p);
                warning(b.s);
                free(b.s);
            }
            if (config_check(e, "property")) use_property(e);
        } else if (strncmp(k, "CONSTRAINT", 10) == 0) {
            e = config_call(it->name, "constraint", 'r');
            if (config_check(e, "constraint")) add_check(&constraints, &nconstraints, e->def, e, e->def->name);
        } else if (strncmp(k, "ACTION_CONSTRAINT", 17) == 0) {
            e = config_call(it->name, "action constraint", 'r');
            if (config_check(e, "action constraint")) add_check(&action_constraints, &naction_constraints, e->def, e, e->def->name);
        } else if (strcmp(k, "VIEW") == 0) {
            view_expr = config_call(it->name, "view function", 'r');
            if (!literal_def(view_expr) && constant_value(view_expr)) config_error("The view function %s cannot be a constant.", S(it->name));
        }
        else if (strcmp(k, "SYMMETRY") == 0) {
            /* As TLC: an undefined name is reported, any other error as no set of functions. */
            sigjmp_buf here;
            handler = &here;
            if (sigsetjmp(here, 0)) {
                handler = NULL;
                if (error_status == TPP_ERROR_CONFIG) { print_error_message(); exit(TPP_ERROR_CONFIG); }
                print_exception("java.lang.RuntimeException", ": ");
                printf("The symmetry operator must specify a set of functions.\n");
                exit(TPP_ERROR);
            }
            Value *g = enumerate(eval(config_call(it->name, "symmetry function", 0), root_ctx));
            handler = NULL;
            for (int j = 0; j < g->n; j++)
                if (!is_fcn(realize(g->u.set.elems[j]))) {
                    print_exception("java.lang.RuntimeException", ": ");
                    printf("The symmetry values must be function records.\n");
                    exit(TPP_ERROR);
                }
            if (!nowarning) warn_symmetry_sets(root_sym(it->name)->def->body, g);
            Value *s = permutation_group(g);
            symmetry = s->u.set.elems;
            nsymmetry = s->n;
            symmetry_tables();
        } else if (strcmp(k, "CHECK_DEADLOCK") == 0) {
            if (it->value->kind == V_BOOL && !it->value->u.i) check_deadlock = 0;
        }
    }
    }
    if (spec && (init || next)) config_error("The configuration file cannot specify both INIT/NEXT and SPECIFICATION fields.");
    if (spec) use_specification(spec);
    if (init) init_expr = init;
    if (next) next_expr = next;
    return 0;
}


/* ---------------------------------------------------------------- liveness */

/* A behavior violates a property when it satisfies the property's
   negation (§10.5). The negation, in negation normal form over state and
   action predicates, is expanded into a tableau; its product with the
   state graph, where every state may stutter, is searched for a strongly
   connected component that the tableau accepts and the fairness
   conditions allow (§10.6). */

enum { TF_ATOM, TF_AND, TF_OR, TF_ALWAYS, TF_EVENTUALLY, TF_TRUE, TF_FALSE };

typedef struct TF {
    int op;
    struct TF *a, *b;
    Expr *atom;        /* TF_ATOM: a predicate, negated when neg */
    Ctx c;
    int neg, action;   /* action: the predicate relates a state to its successor */
    int index;         /* TF_EVENTUALLY: its acceptance set; TF_ATOM: its cache slot */
} TF;

static TF **eventualities;
static int neventualities;
static TF **atoms;
static int natoms;

static TF *tf_new(int op, TF *a, TF *b) {
    if (op == TF_AND || op == TF_OR) {   /* TLC simplifies a junction with a constant */
        int unit = op == TF_AND ? TF_TRUE : TF_FALSE;
        if (a->op == unit) return b;
        if (b->op == unit) return a;
        if (a->op == TF_TRUE + TF_FALSE - unit) return a;
        if (b->op == TF_TRUE + TF_FALSE - unit) return b;
    }
    TF *t = NEW(TF);
    t->op = op; t->a = a; t->b = b;
    if (op == TF_EVENTUALLY) {
        t->index = neventualities;
        eventualities = xrealloc(eventualities, sizeof(TF *) * (size_t)(neventualities + 1));
        eventualities[neventualities++] = t;
    }
    return t;
}

static TF *tf_atom(Expr *e, Ctx c, int neg) {
    TF *t = tf_new(TF_ATOM, NULL, NULL);
    t->atom = e; t->c = c; t->neg = neg;
    t->action = expr_level(e) >= 2;
    t->index = natoms;
    atoms = xrealloc(atoms, sizeof(TF *) * (size_t)(natoms + 1));
    atoms[natoms++] = t;
    return t;
}

static Expr *synth(int kind, Expr *at, int n, Expr *a, Expr *b) {
    Expr *e = NEW(Expr);
    *e = *at;
    e->kind = (short)kind;
    e->n = n;
    e->args = NEWN(Expr *, 2);
    e->args[0] = a;
    e->args[1] = b;
    e->nb = 0;
    return e;
}

static TF *to_tf(Expr *e, Ctx c, int neg);
static Expr *tf_unhandled;   /* the formula to_tf failed on as one it cannot handle */

typedef struct { Expr *body; TF *acc; int op; } TfData;

static int tf_bound_fn(Ctx c, void *data) {
    TfData *d = data;
    TF *t = to_tf(d->body, c, 0);
    d->acc = d->acc ? tf_new(d->op, d->acc, t) : t;
    return 0;
}

/* The formula e (negated when neg) in negation normal form. */
static TF *to_tf(Expr *e, Ctx c, int neg) {
    if (expr_level(e) <= 2) {
        if (expr_level(e) == 0) {   /* as TLC, a constant is its value */
            Value *v = eval(e, c);
            if (v->kind == V_BOOL) return tf_new((v->u.i != 0) != neg ? TF_TRUE : TF_FALSE, NULL, NULL);
        }
        return tf_atom(e, c, neg);
    }
    switch (e->kind) {
    case K_CALL: {
        Def *d = e->def->replacement ? e->def->replacement : e->def;
        if (d->kind != D_OP || d->native) break;
        Env *env = d->local == 2 ? env_find(c.env, d)->ctx_env : NULL;
        for (int i = 0; i < e->n; i++) env = mk_thunk(d->params[i], e->args[i], c, env);
        deeper();
        TF *t = to_tf(d->body, (Ctx){env, c.inst}, neg);
        eval_depth--;
        return t;
    }
    case K_INSTCALL:
        if (e->def->kind != D_OP || e->def->native) break;
        return to_tf(e->def->body, instance_ctx(e, c), neg);
    case K_AND: case K_OR: {
        int op = (e->kind == K_AND) != neg ? TF_AND : TF_OR;
        TF *t = to_tf(e->args[0], c, neg);
        for (int i = 1; i < e->n; i++) t = tf_new(op, t, to_tf(e->args[i], c, neg));
        return t;
    }
    case K_NOT: return to_tf(e->args[0], c, !neg);
    case X_IF: {
        /* IF p THEN A ELSE B is (p /\ A) \/ (~p /\ B), negated in A and B. */
        TF *then = tf_new(TF_AND, to_tf(e->args[0], c, 0), to_tf(e->args[1], c, neg));
        TF *other = tf_new(TF_AND, to_tf(e->args[0], c, 1), to_tf(e->args[2], c, neg));
        return tf_new(TF_OR, then, other);
    }
    case K_IMPLIES: {
        TF *a = to_tf(e->args[0], c, !neg), *b = to_tf(e->args[1], c, neg);
        return tf_new(neg ? TF_AND : TF_OR, a, b);
    }
    case K_EQUIV: {
        TF *both = tf_new(TF_AND, to_tf(e->args[0], c, 0), to_tf(e->args[1], c, neg));
        TF *neither = tf_new(TF_AND, to_tf(e->args[0], c, 1), to_tf(e->args[1], c, !neg));
        return tf_new(TF_OR, both, neither);
    }
    case K_ALWAYS: return tf_new(neg ? TF_EVENTUALLY : TF_ALWAYS, to_tf(e->args[0], c, neg), NULL);
    case K_EVENTUALLY: return tf_new(neg ? TF_ALWAYS : TF_EVENTUALLY, to_tf(e->args[0], c, neg), NULL);
    case K_LEADSTO: {
        /* P ~> Q is [](~P \/ <>Q). */
        Expr *ev = synth(K_EVENTUALLY, e, 1, e->args[1], NULL);
        Expr *body = synth(K_IMPLIES, e, 2, e->args[0], ev);
        return to_tf(synth(K_ALWAYS, e, 1, body, NULL), c, neg);
    }
    case X_WF: case X_SF: {
        /* WF: []<>~ENABLED <<A>>_v \/ []<><<A>>_v; SF: <>[]~ENABLED <<A>>_v \/ []<><<A>>_v. */
        Expr *angle = synth(X_ANGLEACT, e, 2, e->args[1], e->args[0]);
        Expr *en = synth(K_ENABLED, e, 1, angle, NULL);
        TF *step = tf_new(TF_ALWAYS, tf_new(TF_EVENTUALLY, tf_atom(angle, c, 0), NULL), NULL);
        TF *off = e->kind == X_WF ? tf_new(TF_ALWAYS, tf_new(TF_EVENTUALLY, tf_atom(en, c, 1), NULL), NULL)
                                  : tf_new(TF_EVENTUALLY, tf_new(TF_ALWAYS, tf_atom(en, c, 1), NULL), NULL);
        TF *fair = tf_new(TF_OR, off, step);
        if (!neg) return fair;
        /* ~WF: <>[]ENABLED /\ <>[]~step; ~SF: []<>ENABLED /\ <>[]~step. */
        TF *nostep = tf_new(TF_EVENTUALLY, tf_new(TF_ALWAYS, tf_atom(angle, c, 1), NULL), NULL);
        TF *on = e->kind == X_WF ? tf_new(TF_EVENTUALLY, tf_new(TF_ALWAYS, tf_atom(en, c, 0), NULL), NULL)
                                 : tf_new(TF_ALWAYS, tf_new(TF_EVENTUALLY, tf_atom(en, c, 0), NULL), NULL);
        return tf_new(TF_AND, on, nostep);
    }
    case X_FORALL: case X_EXISTS: {
        TfData d = {e->args[0], NULL, (e->kind == X_FORALL) != neg ? TF_AND : TF_OR};
        Expr *body = e->args[0];
        if (neg) body = synth(K_NOT, e, 1, body, NULL);
        d.body = body;
        for_bindings(e->bounds, e->nb, c, tf_bound_fn, &d);
        return d.acc ? d.acc : tf_new(d.op == TF_AND ? TF_TRUE : TF_FALSE, NULL, NULL);
    }
    default: break;
    }
    tf_unhandled = e;
    fail_at(e, "TLC cannot handle this temporal formula.");
}

/* A tableau node: the literals that hold now, the formulas that must hold
   from the next state on, and the eventualities it defers. */
typedef struct { TF **lits; int nlits; TF **next; int nnext; uint64_t pending; int *succ; int nsucc; int expanded; } TNode;

static TNode *tnodes;
static int ntnodes;

typedef struct { TF **v; int n; } TFList;

static void tfl_add(TFList *l, TF *t) {
    for (int i = 0; i < l->n; i++) if (l->v[i] == t) return;
    TF **grown = NEWN(TF *, l->n + 1);
    if (l->n) memcpy(grown, l->v, sizeof(TF *) * (size_t)l->n);
    grown[l->n++] = t;
    l->v = grown;
}

static int tfl_same(TF **a, int na, TF **b, int nb) {
    if (na != nb) return 0;
    for (int i = 0; i < na; i++) {
        int found = 0;
        for (int j = 0; j < nb && !found; j++) found = a[i] == b[j];
        if (!found) return 0;
    }
    return 1;
}

static int tnode_add(TFList lits, TFList next, uint64_t pending) {
    for (int i = 0; i < lits.n; i++)
        for (int j = 0; j < lits.n; j++)
            if (lits.v[i]->atom == lits.v[j]->atom && lits.v[i]->c.env == lits.v[j]->c.env && lits.v[i]->neg != lits.v[j]->neg) return -1;
    for (int i = 0; i < ntnodes; i++)
        if (tnodes[i].pending == pending && tfl_same(tnodes[i].lits, tnodes[i].nlits, lits.v, lits.n) &&
            tfl_same(tnodes[i].next, tnodes[i].nnext, next.v, next.n)) return i;
    tnodes = xrealloc(tnodes, sizeof(TNode) * (size_t)(ntnodes + 1));
    TNode *t = &tnodes[ntnodes];
    memset(t, 0, sizeof *t);
    t->lits = lits.v; t->nlits = lits.n; t->next = next.v; t->nnext = next.n; t->pending = pending;
    return ntnodes++;
}

/* Expands the formulas in todo into tableau nodes, adding their indices to out. */
static void expand(TFList todo, TFList lits, TFList next, uint64_t pending, List *out) {
    if (todo.n == 0) {
        int i = tnode_add(lits, next, pending);
        if (i >= 0) list_add(out, mk_int(i));
        return;
    }
    TF *f = todo.v[todo.n - 1];
    TFList rest = {todo.v, todo.n - 1};
    switch (f->op) {
    case TF_TRUE: expand(rest, lits, next, pending, out); return;
    case TF_FALSE: return;
    case TF_ATOM: { TFList l = lits; tfl_add(&l, f); expand(rest, l, next, pending, out); return; }
    case TF_AND: { TFList r = rest; tfl_add(&r, f->a); tfl_add(&r, f->b); expand(r, lits, next, pending, out); return; }
    case TF_OR: {
        TFList r1 = rest; tfl_add(&r1, f->a); expand(r1, lits, next, pending, out);
        TFList r2 = rest; tfl_add(&r2, f->b); expand(r2, lits, next, pending, out);
        return;
    }
    case TF_ALWAYS: { TFList r = rest; tfl_add(&r, f->a); TFList n = next; tfl_add(&n, f); expand(r, lits, n, pending, out); return; }
    case TF_EVENTUALLY: {
        TFList r1 = rest; tfl_add(&r1, f->a); expand(r1, lits, next, pending, out);
        TFList n = next; tfl_add(&n, f); expand(rest, lits, n, pending | (1ULL << (f->index & 63)), out);
        return;
    }
    }
}

static void tnode_expand(int i) {
    if (tnodes[i].expanded) return;
    tnodes[i].expanded = 1;
    List out = {0};
    TFList todo = {tnodes[i].next, tnodes[i].nnext}, none = {0};
    expand(todo, none, none, 0, &out);
    tnodes[i].succ = NEWN(int, out.n);
    for (int k = 0; k < out.n; k++) tnodes[i].succ[k] = (int)out.v[k]->u.i;
    tnodes[i].nsucc = out.n;
}

/* Cached values of atoms in states and on edges. */
static signed char *state_atom_cache;   /* nstates * natoms: 0 unknown, 1 false, 2 true */
static Value ***decoded;

static Value **state_values(int s) {
    if (!decoded[s]) decoded[s] = decode_state(s);
    return decoded[s];
}

/* t in state s, or on the step from s to the state next. */
static int atom_holds_on(TF *t, int s, Value **next);

/* Whether e holds on the step from cur to next; what it evaluates is
   dropped after, unless the caller's arena is the scratch one. */
static int holds_in(Expr *e, Ctx c, Value **cur, Value **next) {
    Value **saved_cur = cur_state, **saved_next = next_state;
    Arena *saved_arena = arena;
    cur_state = cur;
    next_state = next;
    arena = &scratch_arena;
    int v = eval_bool(e, c);
    cur_state = saved_cur;
    next_state = saved_next;
    arena = saved_arena;
    if (arena != &scratch_arena) arena_reset(&scratch_arena);
    return v;
}

static int atom_holds(TF *t, int s, int to) { return atom_holds_on(t, s, state_values(to)); }

static int atom_holds_on(TF *t, int s, Value **next) {
    int v;
    if (!t->action) {
        signed char *slot = &state_atom_cache[(size_t)s * (size_t)natoms + (size_t)t->index];
        if (!*slot) *slot = (signed char)(holds_in(t->atom, t->c, state_values(s), NULL) ? 2 : 1);
        v = *slot == 2;
    } else {
        v = holds_in(t->atom, t->c, state_values(s), next);
    }
    return v != t->neg;
}

/* Under symmetry a state's successor is kept as the state found first
   with its fingerprint, which may be a permutation of it, so an action
   is evaluated, as TLC does, on the successors themselves: a state's
   successors, regenerated once, with their fingerprints. */
typedef struct { int n; Value ***v; uint64_t *fp; } Successors;
static Successors *real_succ;
static List real_found;

static int real_emit(void) {
    for (int i = 0; i < nvars; i++) if (!gen_target[i]) return 0;
    list_add(&real_found, (Value *)memcpy(NEWN(Value *, nvars), gen_target, sizeof(Value *) * (size_t)nvars));
    return 0;
}

static Successors *real_successors(int s) {
    Successors *r = &real_succ[s];
    if (r->v) return r;
    Value **saved_cur = cur_state, **saved_next = next_state, **saved_target = gen_target;
    int (*saved_emit)(void) = gen_emit;
    Value **st = NEWN(Value *, nvars);
    real_found = (List){0};   /* its storage is the arena's, which a simulation resets */
    gen_emit = real_emit;
    gen_target = st;
    cur_state = state_values(s);
    for (int a = 0; a < nactions; a++) {
        memset(st, 0, sizeof(Value *) * (size_t)nvars);
        next_state = st;
        gen(actions[a].e, actions[a].c, NULL);
    }
    cur_state = saved_cur; next_state = saved_next; gen_target = saved_target; gen_emit = saved_emit;
    r->n = real_found.n;
    r->v = NEWN(Value **, r->n + 1);
    r->fp = NEWN(uint64_t, r->n + 1);
    for (int i = 0; i < r->n; i++) { r->v[i] = (Value **)real_found.v[i]; r->fp[i] = fingerprint(r->v[i]); }
    return r;
}

static int tnode_admits(int n, int s) {
    for (int i = 0; i < tnodes[n].nlits; i++) {
        TF *t = tnodes[n].lits[i];
        if (!t->action && !atom_holds(t, s, s)) return 0;
    }
    return 1;
}

/* Whether the step from s to `to` can satisfy tableau node n's actions:
   under symmetry, some step of s to a state with to's fingerprint, or
   s stuttering, does. */
static int tnode_step(int n, int s, int to) {
    if (!nsymmetry) {
        for (int i = 0; i < tnodes[n].nlits; i++) {
            TF *t = tnodes[n].lits[i];
            if (t->action && !atom_holds(t, s, to)) return 0;
        }
        return 1;
    }
    Successors *r = real_successors(s);
    for (int k = -1; k < r->n; k++) {
        if (k < 0 ? to != s : r->fp[k] != states[to].fp) continue;
        Value **next = k < 0 ? state_values(s) : r->v[k];
        int ok = 1;
        for (int i = 0; i < tnodes[n].nlits && ok; i++) {
            TF *t = tnodes[n].lits[i];
            ok = !t->action || atom_holds_on(t, s, next);
        }
        if (ok) return 1;
    }
    return 0;
}

/* The product graph: nodes are (state, tableau node). */
typedef struct { int state, tnode; int *succ, *act; int nsucc; int index, low, onstack, mark; } PNode;

static PNode *pnodes;
static int npnodes, cappnodes;
/* Each state's product nodes, as (tableau node, product node) pairs: a
   state has few, and the successors of a step share its target's. */
typedef struct { int tnode, node; } PRef;
typedef struct { PRef *v; int n; } PRefs;
static PRefs *pnodes_of;

static int pnode_find(int s, int t, int create) {
    PRefs *l = &pnodes_of[s];
    for (int i = 0; i < l->n; i++) if (l->v[i].tnode == t) return l->v[i].node;
    if (!create) return -1;
    PUSH(l->v, l->n, ((PRef){t, npnodes}));
    if (npnodes == cappnodes) {
        cappnodes = cappnodes ? cappnodes * 2 : 1024;
        pnodes = xrealloc(pnodes, sizeof(PNode) * (size_t)cappnodes);
    }
    PNode *p = &pnodes[npnodes];
    memset(p, 0, sizeof *p);
    p->state = s; p->tnode = t; p->index = -1;
    return npnodes++;
}


/* The successors of a product node by the n steps of its state in edges,
   (target, action) pairs, then the stuttering step if stutter is set, with
   each successor of the tableau node. */
static void pnode_steps(int p, const int *edges, int n, int stutter) {
    static int *found, cap;   /* (successor, action) pairs */
    int s = pnodes[p].state, t = pnodes[p].tnode, m = 0;
    tnode_expand(t);
    for (int e = 0; e < n + stutter; e++) {
        int target = e < n ? edges[2 * e] : s, action = e < n ? edges[2 * e + 1] : STUTTER;
        if (!tnode_step(t, s, target)) continue;
        for (int k = 0; k < tnodes[t].nsucc; k++) {
            int u = tnodes[t].succ[k];
            if (!tnode_admits(u, target)) continue;
            if (2 * m + 2 > cap) found = xrealloc(found, sizeof(int) * (size_t)(cap = 2 * cap + 16));
            found[2 * m] = pnode_find(target, u, 1);
            found[2 * m + 1] = action;
            m++;
        }
    }
    PNode *pn = &pnodes[p];
    pn->succ = alloc_raw(sizeof(int) * (size_t)(2 * m));
    pn->act = pn->succ + m;
    for (int i = 0; i < m; i++) { pn->succ[i] = found[2 * i]; pn->act[i] = found[2 * i + 1]; }
    pn->nsucc = m;
}

/* Each edge of the state graph, and the stuttering step unless an edge is
   one. A state the search stopped before expanding (TLCSet("exit")) has
   none, as TLC's graph has no such node. */
static void pnode_successors(int p) {
    StateRec *r = &states[pnodes[p].state];
    if (exit_requested && pnodes[p].state >= expanded) { pnode_steps(p, NULL, 0, 0); return; }
    int self = 0;
    for (int e = 0; e < r->nedges; e++) self |= r->edges[2 * e] == pnodes[p].state;
    pnode_steps(p, r->edges, r->nedges, !self);
}

/* The product as TLC builds it from a behavior, as its states arrive: a
   node's successors are those of the first position that expands it, its
   state taking the behavior's next step or stuttering; the last state
   only stutters. A node never expanded has none. */
static void sim_product(void) {
    for (int i = 0; i < sim_n; i++) {
        int s = sim_order[i], next = i + 1 < sim_n && sim_order[i + 1] != s;
        int step[2] = {next ? sim_order[i + 1] : s, next ? sim_acts[i + 1] : STUTTER};
        for (int p = 0; p < npnodes; p++)
            if (pnodes[p].state == s && !pnodes[p].succ) pnode_steps(p, step, next, 1);
    }
    for (int p = 0; p < npnodes; p++)
        if (!pnodes[p].succ) pnode_steps(p, NULL, 0, 0);
}

/* Fairness of the specification: WF_v(A) or SF_v(A), as an enabledness
   predicate on states and a step predicate on edges. */
typedef struct { int strong; Expr *enabled, *step; Ctx c; } Fair;

static Fair *fairs;
static int nfairs;

static int fair_enabled(Fair *f, int s) { return holds_in(f->enabled, f->c, state_values(s), next_state); }

static int fair_step(Fair *f, int s, int to) {
    if (to == STUTTER) return 0;
    return holds_in(f->step, f->c, state_values(s), state_values(to));
}

static void collect_fairness(Expr *e, Ctx c);

static int fairness_bound_fn(Ctx c, void *body) {
    collect_fairness(body, c);
    return 0;
}

static void collect_fairness(Expr *e, Ctx c) {
    e = unfold(e);
    if (e->kind == K_AND) { for (int i = 0; i < e->n; i++) collect_fairness(e->args[i], c); return; }
    if (e->kind == X_FORALL) { for_bindings(e->bounds, e->nb, c, fairness_bound_fn, e->args[0]); return; }
    if (e->kind == X_WF || e->kind == X_SF) {
        Expr *angle = synth(X_ANGLEACT, e, 2, e->args[1], e->args[0]);
        fairs = xrealloc(fairs, sizeof(Fair) * (size_t)(nfairs + 1));
        fairs[nfairs].strong = e->kind == X_SF;
        fairs[nfairs].enabled = synth(K_ENABLED, e, 1, angle, NULL);
        fairs[nfairs].step = angle;
        fairs[nfairs].c = c;
        nfairs++;
        return;
    }
    if (e->kind == K_CALL && e->def->kind == D_OP && !e->def->native) {
        Env *env = NULL;
        for (int i = 0; i < e->n; i++) env = mk_thunk(e->def->params[i], e->args[i], c, env);
        collect_fairness(e->def->body, (Ctx){env, c.inst});
        return;
    }
    if (e->kind == K_INSTCALL && e->def->kind == D_OP && !e->def->native) {
        collect_fairness(e->def->body, instance_ctx(e, c));
        return;
    }
    fail_at(e, "TLC cannot handle the fairness condition.");
}

static int accept_component(int *nodes, int n);

/* Tarjan's algorithm over the product nodes reachable from roots whose
   mark equals `mark`; each component goes to accept_component in
   completion order. Returns the first accepted node or -1. */
/* The product grows as it is explored; the stacks hold at most every node. */
static void grow_stacks(int **stack, int **cstack, int **cedge, int *cap) {
    if (*cap > npnodes) return;
    *cap = npnodes * 2 + 16;
    *stack = xrealloc(*stack, sizeof(int) * (size_t)*cap);
    *cstack = xrealloc(*cstack, sizeof(int) * (size_t)*cap);
    *cedge = xrealloc(*cedge, sizeof(int) * (size_t)*cap);
}

static int scc_search(int *roots, int nroots, int mark) {
    int index = 0, found = -1, sp = 0, cap = 0;
    int *stack = NULL, *cstack = NULL, *cedge = NULL;
    for (int r = 0; r < nroots && found < 0; r++) {
        int root = roots[r];
        if (pnodes[root].index >= 0 || pnodes[root].mark != mark) continue;
        grow_stacks(&stack, &cstack, &cedge, &cap);
        int cp = 0;
        cstack[cp] = root; cedge[cp] = 0; cp++;
        pnodes[root].index = pnodes[root].low = index++;
        stack[sp++] = root;
        pnodes[root].onstack = 1;
        while (cp > 0 && found < 0) {
            int v = cstack[cp - 1];
            if (!pnodes[v].succ) pnode_successors(v);
            grow_stacks(&stack, &cstack, &cedge, &cap);
            if (cedge[cp - 1] < pnodes[v].nsucc) {
                int w = pnodes[v].succ[cedge[cp - 1]++];
                if (pnodes[w].mark != mark) continue;
                if (pnodes[w].index < 0) {
                    pnodes[w].index = pnodes[w].low = index++;
                    stack[sp++] = w;
                    pnodes[w].onstack = 1;
                    cstack[cp] = w; cedge[cp] = 0; cp++;
                } else if (pnodes[w].onstack && pnodes[w].index < pnodes[v].low) {
                    pnodes[v].low = pnodes[w].index;
                }
                continue;
            }
            cp--;
            if (cp > 0) {
                int u = cstack[cp - 1];
                if (pnodes[v].low < pnodes[u].low) pnodes[u].low = pnodes[v].low;
            }
            if (pnodes[v].low == pnodes[v].index) {
                int start = sp;
                do start--; while (stack[start] != v);
                int n = sp - start;
                int *comp = xmalloc(sizeof(int) * (size_t)n);
                memcpy(comp, stack + start, sizeof(int) * (size_t)n);
                for (int i = 0; i < n; i++) pnodes[comp[i]].onstack = 0;
                sp = start;
                found = accept_component(comp, n);
                free(comp);
            }
        }
    }
    free(stack); free(cstack); free(cedge);
    return found;
}

static int in_set(int *nodes, int n, int x) {
    for (int i = 0; i < n; i++) if (nodes[i] == x) return 1;
    return 0;
}

static int *accepted;   /* the accepted component */
static int naccepted;

static int accept_component(int *nodes, int n) {
    /* A component must have an edge inside it. */
    int internal = 0;
    for (int i = 0; i < n && !internal; i++)
        for (int k = 0; k < pnodes[nodes[i]].nsucc && !internal; k++) internal = in_set(nodes, n, pnodes[nodes[i]].succ[k]);
    if (!internal) return -1;
    for (int e = 0; e < neventualities; e++) {
        int ok = 0;
        for (int i = 0; i < n && !ok; i++) ok = !(tnodes[pnodes[nodes[i]].tnode].pending & (1ULL << (e & 63)));
        if (!ok) return -1;
    }
    for (int f = 0; f < nfairs; f++) {
        Fair *fr = &fairs[f];
        int step = 0, disabled = 0, enabled_any = 0;
        for (int i = 0; i < n && !step; i++)
            for (int k = 0; k < pnodes[nodes[i]].nsucc && !step; k++)
                if (in_set(nodes, n, pnodes[nodes[i]].succ[k]))
                    step = fair_step(fr, pnodes[nodes[i]].state, pnodes[nodes[i]].act[k] == STUTTER ? STUTTER : pnodes[pnodes[nodes[i]].succ[k]].state);
        if (step) continue;
        for (int i = 0; i < n; i++) {
            if (fair_enabled(fr, pnodes[nodes[i]].state)) enabled_any = 1;
            else disabled = 1;
        }
        if (!fr->strong) { if (disabled) continue; return -1; }
        if (!enabled_any) continue;
        /* Strong fairness: drop the states where the action is enabled and
           look for components among the rest. */
        int mark = f + 1;
        int *roots = xmalloc(sizeof(int) * (size_t)n);
        int nroots = 0;
        for (int i = 0; i < n; i++)
            if (!fair_enabled(fr, pnodes[nodes[i]].state)) { pnodes[nodes[i]].mark = mark; roots[nroots++] = nodes[i]; }
        int found = -1;
        if (nroots) {
            /* The outer search is done with these nodes; search them afresh. */
            int *saved = xmalloc(sizeof(int) * (size_t)nroots);
            for (int i = 0; i < nroots; i++) { saved[i] = pnodes[roots[i]].index; pnodes[roots[i]].index = -1; }
            found = scc_search(roots, nroots, mark);
            for (int i = 0; i < nroots; i++) pnodes[roots[i]].index = saved[i];
            free(saved);
        }
        for (int i = 0; i < nroots; i++) pnodes[roots[i]].mark = 0;
        free(roots);
        return found;
    }
    accepted = xmalloc(sizeof(int) * (size_t)n);
    memcpy(accepted, nodes, sizeof(int) * (size_t)n);
    naccepted = n;
    return nodes[0];
}

/* The shortest path in the product from any of `from` to `to`, within
   nodes accepted by `allowed` (all if NULL): nodes and the actions of the
   edges into them. */
static int product_path(int *from, int nfrom, int to, int *allowed, int nallowed, List *path, List *acts) {
    int n = npnodes;   /* expanding a node may add nodes, which the path does not go through */
    int *prev = xmalloc(sizeof(int) * (size_t)n);
    int *pact = xmalloc(sizeof(int) * (size_t)n);
    int *queue = xmalloc(sizeof(int) * (size_t)n);
    for (int i = 0; i < n; i++) prev[i] = -2;
    int qh = 0, qt = 0;
    for (int i = 0; i < nfrom; i++) if (prev[from[i]] == -2) { prev[from[i]] = -1; pact[from[i]] = -1; queue[qt++] = from[i]; }
    int found = 0;
    while (qh < qt) {
        int v = queue[qh++];
        if (v == to) { found = 1; break; }
        if (!pnodes[v].succ) pnode_successors(v);
        for (int k = 0; k < pnodes[v].nsucc; k++) {
            int w = pnodes[v].succ[k];
            if (w >= n || prev[w] != -2) continue;
            if (allowed && !in_set(allowed, nallowed, w)) continue;
            prev[w] = v;
            pact[w] = pnodes[v].act[k];
            queue[qt++] = w;
        }
    }
    if (found) {
        int n = 0;
        for (int v = to; v >= 0; v = prev[v]) n++;
        int *rev = xmalloc(sizeof(int) * (size_t)n);
        int k = n;
        for (int v = to; v >= 0; v = prev[v]) rev[--k] = v;
        for (int i = 0; i < n; i++) { list_add(path, mk_int(rev[i])); list_add(acts, mk_int(pact[rev[i]])); }
        free(rev);
    }
    free(prev); free(pact); free(queue);
    return found;
}

/* State number of a counterexample, state s, after the state prev (-1 for none). */
static void print_live_state(int number, int s, int action, int prev) {
    print_numbered(number, action, state_values(s), prev < 0 ? NULL : state_values(prev));
}

/* Prints a lasso through the accepted component: the shortest path to it,
   then a cycle that meets each eventuality and fairness condition. */
static void print_counterexample(int *inits, int ninits) {
    List path = {0}, acts = {0};
    /* entry: the accepted node nearest an initial node */
    int entry = -1;
    {
        int *prev = xmalloc(sizeof(int) * (size_t)npnodes);
        int *queue = xmalloc(sizeof(int) * (size_t)npnodes);
        for (int i = 0; i < npnodes; i++) prev[i] = -2;
        int qh = 0, qt = 0;
        for (int i = 0; i < ninits; i++) if (prev[inits[i]] == -2) { prev[inits[i]] = -1; queue[qt++] = inits[i]; }
        while (qh < qt && entry < 0) {
            int v = queue[qh++];
            if (in_set(accepted, naccepted, v)) { entry = v; break; }
            for (int k = 0; k < pnodes[v].nsucc; k++) {
                int w = pnodes[v].succ[k];
                if (prev[w] != -2) continue;
                prev[w] = v;
                queue[qt++] = w;
            }
        }
        free(prev); free(queue);
    }
    product_path(inits, ninits, entry, NULL, 0, &path, &acts);
    printf("Error: Temporal properties were violated.\n\n");
    printf("Error: The following behavior constitutes a counter-example:\n\n");
    int number = 0;
    for (int i = 0; i < path.n; i++) {
        int p = (int)path.v[i]->u.i;
        int act = (int)acts.v[i]->u.i;
        print_live_state(++number, pnodes[p].state, i == 0 ? -1 : act, i == 0 ? -1 : pnodes[path.v[i - 1]->u.i].state);
    }
    int entry_number = number;
    /* The cycle: from the entry through each required node or edge, back. */
    List cyc = {0}, cacts = {0};
    int cur = entry;
    for (int e = 0; e < neventualities; e++) {
        int target = -1;
        for (int i = 0; i < naccepted && target < 0; i++)
            if (!(tnodes[pnodes[accepted[i]].tnode].pending & (1ULL << (e & 63)))) target = accepted[i];
        if (target >= 0 && target != cur) {
            List p = {0}, a = {0};
            product_path(&cur, 1, target, accepted, naccepted, &p, &a);
            for (int i = 1; i < p.n; i++) { list_add(&cyc, p.v[i]); list_add(&cacts, a.v[i]); }
            cur = target;
        }
    }
    for (int f = 0; f < nfairs; f++) {
        Fair *fr = &fairs[f];
        int from = -1, to = -1, act = 0;
        for (int i = 0; i < naccepted && from < 0; i++)
            for (int k = 0; k < pnodes[accepted[i]].nsucc && from < 0; k++) {
                int w = pnodes[accepted[i]].succ[k];
                if (in_set(accepted, naccepted, w) && pnodes[accepted[i]].act[k] != STUTTER &&
                    fair_step(fr, pnodes[accepted[i]].state, pnodes[w].state)) { from = accepted[i]; to = w; act = pnodes[accepted[i]].act[k]; }
            }
        if (from < 0) continue;
        if (from != cur) {
            List p = {0}, a = {0};
            product_path(&cur, 1, from, accepted, naccepted, &p, &a);
            for (int i = 1; i < p.n; i++) { list_add(&cyc, p.v[i]); list_add(&cacts, a.v[i]); }
        }
        list_add(&cyc, mk_int(to));
        list_add(&cacts, mk_int(act));
        cur = to;
    }
    /* Back to the entry. */
    int stutter_only = 0;
    if (cur == entry && cyc.n == 0) {
        /* a step that returns to the state, by any action, is TLC's stuttering */
        for (int k = 0; k < pnodes[entry].nsucc; k++) if (pnodes[entry].succ[k] == entry) stutter_only = 1;
        if (!stutter_only) {
            /* leave the entry along any edge inside the component */
            for (int k = 0; k < pnodes[entry].nsucc; k++)
                if (in_set(accepted, naccepted, pnodes[entry].succ[k])) {
                    list_add(&cyc, mk_int(pnodes[entry].succ[k]));
                    list_add(&cacts, mk_int(pnodes[entry].act[k]));
                    cur = pnodes[entry].succ[k];
                    break;
                }
        }
    }
    if (stutter_only) {
        printf("State %d: Stuttering\n", number + 1);
        return;
    }
    if (cur != entry) {
        List p = {0}, a = {0};
        int *allowed = accepted;
        product_path(&cur, 1, entry, allowed, naccepted, &p, &a);
        for (int i = 1; i < p.n; i++) { list_add(&cyc, p.v[i]); list_add(&cacts, a.v[i]); }
    }
    for (int i = 0; i + 1 < cyc.n; i++) {
        int p = (int)cyc.v[i]->u.i;
        int before = (int)(i ? cyc.v[i - 1] : path.v[path.n - 1])->u.i;
        print_live_state(++number, pnodes[p].state, (int)cacts.v[i]->u.i, pnodes[before].state);
    }
    printf("Back to state %d: ", entry_number);
    action_label((int)cacts.v[cyc.n - 1]->u.i);
    printf("\n\n");
}

/* Whether TLC checks a formula with a tableau: it has a temporal part
   other than []<>P and <>[]P. */
static int tf_tableau(TF *f) {
    switch (f->op) {
    case TF_AND: case TF_OR: return tf_tableau(f->a) || tf_tableau(f->b);
    case TF_ALWAYS: case TF_EVENTUALLY:
        return f->a->op == f->op || (f->a->op != TF_ALWAYS && f->a->op != TF_EVENTUALLY) || f->a->a->op != TF_ATOM;
    default: return 0;
    }
}

static int check_property(TF *negated) {
    List init = {0};
    TFList todo = {0}, empty = {0};
    tfl_add(&todo, negated);
    expand(todo, empty, empty, 0, &init);
    List roots = {0};
    for (int s = 0; s < nstates; s++) {
        if (states[s].parent != -1) continue;
        for (int i = 0; i < init.n; i++) {
            int t = (int)init.v[i]->u.i;
            if (tnode_admits(t, s)) list_add(&roots, mk_int(pnode_find(s, t, 1)));
        }
    }
    int *r = xmalloc(sizeof(int) * (size_t)(roots.n + 1));
    for (int i = 0; i < roots.n; i++) r[i] = (int)roots.v[i]->u.i;
    if (sim_n && tf_tableau(negated)) sim_product();
    if (scc_search(r, roots.n, 0) >= 0) {
        print_counterexample(r, roots.n);
        return 1;
    }
    return 0;
}

/* TLC checks []<>c and <>[]c, joined at the top, without its tableau;
   in the tableau a constant is never evaluated, so FALSE there is TRUE. */
static TF *tableau_constants(TF *t, int top) {
    switch (t->op) {
    case TF_FALSE: return top ? t : tf_new(TF_TRUE, NULL, NULL);
    case TF_AND: case TF_OR:
        t->a = tableau_constants(t->a, top);
        t->b = tableau_constants(t->b, top);
        return t;
    case TF_ALWAYS: case TF_EVENTUALLY:
        if (top && (t->a->op == TF_ALWAYS || t->a->op == TF_EVENTUALLY) && t->a->op != t->op &&
            (t->a->a->op == TF_TRUE || t->a->a->op == TF_FALSE)) return t;
        t->a = tableau_constants(t->a, 0);
        return t;
    default: return t;
    }
}

/* What TLC evaluates of the properties on each initial state, before its
   invariants: the literals of each initial node of a tableau, up to a
   false one; without a tableau, each state predicate but ENABLED. */
static TFList *init_lits;
static int ninit_lits;

static void add_init_lits(TF **v, int n) {
    TFList l = {NEWN(TF *, n), 0};
    for (int i = 0; i < n; i++) if (!v[i]->action) l.v[l.n++] = v[i];
    init_lits = xrealloc(init_lits, sizeof(TFList) * (size_t)(ninit_lits + 1));
    init_lits[ninit_lits++] = l;
}

static void live_init_lits(void) {
    for (int p = 0; p < ntemporal_props; p++) {
        natoms = 0;
        neventualities = 0;
        ntnodes = 0;
        TF *neg = tableau_constants(to_tf(temporal_props[p].expr, root_ctx, 1), 1);
        if (!tf_tableau(neg)) {
            for (int i = 0; i < natoms; i++)
                if (atoms[i]->atom->kind != K_ENABLED) add_init_lits(&atoms[i], 1);
            continue;
        }
        List init = {0};
        TFList todo = {0}, empty = {0};
        tfl_add(&todo, neg);
        expand(todo, empty, empty, 0, &init);
        for (int i = 0; i < init.n; i++) {
            TNode *n = &tnodes[init.v[i]->u.i];
            add_init_lits(n->lits, n->nlits);
        }
    }
}

/* A node's literals come last first: TLC takes them in the formula's order. */
static void eval_init_lits(Value **st) {
    for (int i = 0; i < ninit_lits; i++)
        for (int j = init_lits[i].n - 1; j >= 0; j--) {
            TF *t = init_lits[i].v[j];
            if (holds_in(t->atom, t->c, st, NULL) == t->neg) break;
        }
}

/* TLC reports the first error there and goes on; a second ends the
   search. */
static void live_init(Value **st) {
    if (live_init_failed) { eval_init_lits(st); return; }
    EvalState saved = eval_save();
    sigjmp_buf here;
    handler = &here;
    if (sigsetjmp(here, 0)) {
        eval_restore(saved);
        print_error_message();
        if (error_module != 1) {   /* as for Init: not for a standard module's operator */
            printf("While working on the initial state:\n");
            print_state(st);
            printf("\n");
        }
        live_init_failed = 1;
        return;
    }
    eval_init_lits(st);
    eval_restore(saved);
}

/* An error checking the temporal properties, reported as TLC reports it:
   TLC evaluates their predicates as it expands each state: ENABLED on
   the state, other state predicates on each successor, the actions on
   each step and on stuttering. It reports the first error then, as
   unexpected, with the counts of that moment. */
static int live_error(void) {
    natoms = 0;
    neventualities = 0;
    for (int p = 0; p < ntemporal_props; p++) to_tf(temporal_props[p].expr, root_ctx, 1);
    static int s, seen;   /* static: read after an error jumps back */
    sigjmp_buf here;
    handler = &here;
    if (sigsetjmp(here, 0)) {
        handler = NULL;
        sim_exception();
        printf("%ld states generated, %d distinct states found, %d states left on queue.\n", gen_after[s], seen + 1, seen - s);
        printf("The depth of the complete state graph search is %d.\n", states[seen].depth);
        return TPP_ERROR;
    }
    seen = 0;
    for (s = 0; s < nstates; s++) {
        StateRec *r = &states[s];
        for (int e = 0; e < r->nedges; e++) if (r->edges[2 * e] > seen) seen = r->edges[2 * e];
        for (int e = 0; e <= r->nedges; e++) {
            int t = e < r->nedges ? r->edges[2 * e] : s;   /* the last, stuttering */
            for (int i = 0; i < natoms; i++) {
                TF *a = atoms[i];
                int on_s = a->atom->kind == K_ENABLED;
                if (a->action) holds_in(a->atom, a->c, state_values(s), state_values(t));
                else if (on_s ? e == r->nedges : e < r->nedges) holds_in(a->atom, a->c, state_values(on_s ? s : t), NULL);
            }
            for (int i = 0; i < nfairs; i++) {
                if (e == r->nedges) holds_in(fairs[i].enabled, fairs[i].c, state_values(s), NULL);
                holds_in(fairs[i].step, fairs[i].c, state_values(s), state_values(t));
            }
        }
    }
    /* Not found so: as it came. */
    handler = NULL;
    print_error_message();
    return TPP_FAILURE_EVAL;
}

static void collect_fairnesses(void) {
    for (int i = 0; i < nfairness; i++) collect_fairness(fairness[i], root_ctx);
}

/* Whether a temporal property fails on the state graph; if so, its
   counterexample is printed. */
static int live_violated(void) {
    free(decoded);
    decoded = calloc((size_t)nstates, sizeof(Value **));
    free(real_succ);
    real_succ = calloc((size_t)nstates, sizeof(Successors));
    for (int p = 0; p < ntemporal_props; p++) {
        neventualities = 0;
        natoms = 0;
        ntnodes = 0;
        npnodes = 0;
        free(pnodes_of);
        pnodes_of = calloc((size_t)nstates, sizeof(PRefs));
        TF *neg = tableau_constants(to_tf(temporal_props[p].expr, root_ctx, 1), 1);
        free(state_atom_cache);
        state_atom_cache = calloc((size_t)nstates * (size_t)(natoms + 1), 1);
        if (check_property(neg)) return 1;
    }
    return 0;
}

/* As TLC does before it checks a model: a property that is no formula it
   can check, such as \EE x : F, is an error. Other errors are left for
   the check. */
/* Whether a formula is FALSE whatever the behavior, its constants alone
   deciding it. */
static int tf_false(TF *t) {
    switch (t->op) {
    case TF_FALSE: return 1;
    case TF_AND: return tf_false(t->a) || tf_false(t->b);
    case TF_OR: return tf_false(t->a) && tf_false(t->b);
    case TF_EVENTUALLY: return tf_false(t->a);   /* TLC does not fold []FALSE */
    default: return 0;
    }
}

/* The tableau of ~e, or NULL if building it fails; tf_unhandled names
   what TLC cannot handle. */
static TF *negation(Expr *e) {
    EvalState s = eval_save();
    sigjmp_buf here;
    handler = &here;
    tf_unhandled = NULL;
    TF *volatile negated = NULL;
    if (!sigsetjmp(here, 0)) negated = to_tf(e, root_ctx, 1);
    eval_restore(s);
    return negated;
}

/* simulating: TLC finds no tautology then. */
static int check_temporal_forms(int simulating) {
    int temporal = 0, unsatisfiable = 0;
    for (int p = 0; p < ntemporal_props; p++) {
        TF *negated = negation(temporal_props[p].expr);
        if (tf_unhandled) {
            Buf b = {0};
            loc_text(&b, tf_unhandled);
            printf("Error: TLC cannot handle the temporal formula %s\n", b.s);
            return TPP_ERROR;
        }
        if (expr_level(temporal_props[p].expr) == 3) {   /* a predicate on the initial states is no tautology */
            temporal++;
            unsatisfiable += negated && tf_false(negated);
        }
    }
    if (temporal && unsatisfiable == temporal && !simulating) {
        printf("Error: Temporal formula is a tautology (its negation is unsatisfiable).\n");
        return TPP_FAILURE_LIVENESS_EVAL;
    }
    return 0;
}

/* ---------------------------------------------------------------- driver */

/* The literal TLC fails on first as it processes its constants: the
   definitions of a module, its inner modules, its assumptions, its
   theorems (§11.3). */
static Expr *odd_literal(Module *m) {
    if (m->odd[0]) return m->odd[0];
    for (int i = 0; i < m->nnested; i++) {
        Expr *e = odd_literal(m->nested[i]);
        if (e) return e;
    }
    return m->odd[1] ? m->odd[1] : m->odd[2];
}

int tpp_check(const TPPOptions *o) {
    printf("tpp version %s\n", TPP_VERSION);
    size_t len = strlen(o->spec);
    char *path = xmalloc(len + 5);
    strcpy(path, o->spec);
    if (len < 4 || strcmp(path + len - 4, ".tla") != 0) strcat(path, ".tla");
    char *slash = strrchr(path, '/');
    spec_dir = xmalloc(strlen(path) + 1);
    if (slash) { memcpy(spec_dir, path, (size_t)(slash - path + 1)); spec_dir[slash - path + 1] = 0; }
    else spec_dir[0] = 0;
    char *config_path;
    if (o->config) {
        config_path = (char *)o->config;
    } else {
        config_path = xmalloc(strlen(path) + 1);
        strcpy(config_path, path);
        strcpy(config_path + strlen(config_path) - 4, ".cfg");
    }
    char *config_text = read_file(config_path);
    if (!config_text) config_error("Cannot open the configuration file %s.", config_path);
    read_config(config_text, 1);
    char *text = read_file(path);
    if (!text) {
        printf("Error: Cannot find source file %s.\n", path);
        printf("Error: Parsing or semantic analysis failed.\n");
        return TPP_ERROR_PARSE;
    }

    for (int i = 0; builtin_names[i]; i++) intern(builtin_names[i]);
    for (int i = -1; i < 1023; i++) {
        Value *v = NEW(Value);
        v->kind = V_INT;
        v->u.i = i;
        small_ints[i + 1] = v;
    }
    for (int i = 0; i < 64; i++) index_hash[i] = fmix(atom_word(V_INT, i + 1));
    boolean_set = mk_set(2);
    boolean_set->u.set.elems[0] = FALSEV;
    boolean_set->u.set.elems[1] = TRUEV;
    check_deadlock = o->check_deadlock;
    nowarning = o->nowarning;
    diff_trace = o->difftrace;
    view_trace = o->view;
    keep_going = o->keep_going;
    if (user_file) fclose(user_file);   /* each write is flushed: a run may end anywhere */
    user_file = NULL;
    if (o->user_file && !(user_file = fopen(o->user_file, "w"))) {
        fprintf(stderr, "tpp: cannot write %s\n", o->user_file);
        return TPP_ERROR;
    }

    root = parse_text(text, path);
    char *base = strndup(path + strlen(spec_dir), strlen(path) - strlen(spec_dir) - 4);
    check_file_name(root, base);
    free(base);
    modules = xrealloc(modules, sizeof(Module *) * (size_t)(nmodules + 1));
    modules[nmodules++] = root;
    load_references(root);
    finish_module(root);
    resolve_module(root);
    for (int i = 0; i < nfinished; i++) {
        Expr *e = finished[i]->outer ? NULL : odd_literal(finished[i]);
        if (e && e->num < 0) config_error("TLC can't handle real numbers.\n%s", S(e->op));
        if (e) {
            printf("Error: TLC can't handle a number this big.\n%s\n", S(e->op));
            return TPP_ERROR;
        }
    }
    bind_natives();
    if (!o->simulate) jr_seed(&draw_model, draw_seed = o->seed);   /* TLC: random without -seed */
    starting = 1;
    read_config(config_text, 0);
    setup_model();

    for (int i = 0; i < nfinished; i++) evaluate_constants(finished[i]);
    starting = 0;
    started = time(NULL);
    if (!o->simulate && !nowarning && ntemporal_props) {   /* as TLC's checker starts, before the assumptions */
        if (nsymmetry)
            warning("Declaring symmetry during liveness checking is dangerous. It might cause TLC to miss "
                    "violations of the stated liveness properties. Please check liveness without symmetry defined.");
    }
    int status = 0;
    for (int i = 0; i < nmodules && !status; i++) {
        Module *m = modules[i];
        int used = m == root;
        for (int k = 0; k < root->nextends && !used; k++) used = load_module(root->extends[k], root) == m;
        if (used && m != root) check_assumptions(m, &status);
    }
    if (!status) check_assumptions(root, &status);
    if (status) return status;
    if (!init_expr) {
        if (nvars || ninvariants || nimplied_inits || naction_props || ntemporal_props)
            config_error("The configuration file did not specify the initial state predicate.\n"
                         "Can also be caused by trying to run TLC on a specification from\n"
                         "a module imported with a parameterized INSTANCE statement.");
        if (o->simulate) {   /* TLC's simulator wants an initial state */
            printf("Computed 0 initial states...\n");
            printf("Error: There is no state satisfying the initial state predicate.\n");
            return TPP_ERROR;
        }
        printf("Finished computing initial states: 0 distinct states generated.\n");
        printf("Model checking completed. No error has been found.\n");
        print_counts(0);
        return TPP_OK;
    }
    /* With nothing to check on steps, TLC computes the initial states first. */
    if (!next_expr && (ninvariants || naction_props || ntemporal_props))
        config_error("The configuration file did not specify the next state predicate.");
    arena = &perm_arena;
    sigjmp_buf here;
    handler = &here;
    if (sigsetjmp(here, 0)) {
        handler = NULL;
        print_error_message();
        return TPP_FAILURE_EVAL;
    }
    if (next_expr) split_action(next_expr, root_ctx, next_expr->kind == K_CALL || next_expr->kind == K_INSTCALL ? next_expr->def->name : intern("Action"));
    handler = NULL;
    if (o->dump && !o->simulate) {
        if (!(dump_file = fopen(o->dump, "w"))) { fprintf(stderr, "tpp: cannot write %s\n", o->dump); return TPP_ERROR; }
        fputs("strict digraph DiskGraph {\nnode [shape=box,style=rounded]\nnodesep=0.35;\nsubgraph cluster_graph {\ncolor=\"white\";\n", dump_file);
        dump_actions = o->dump_actions;
    }
    /* A simulation is TLC's with one worker; -continue reports in the search's order. */
    nworkers = o->workers > 1 && !o->simulate && !o->keep_going ? o->workers : 1;
    status = check_temporal_forms(o->simulate);
    if (!status) status = o->simulate ? simulate(o) : search();
    handler = NULL;
    if (dump_file) dump_close();
    stop_workers();
    return status;
}

#ifndef TPP_NO_MAIN
int main(int argc, char **argv) {
    TPPOptions o = {0};
    o.check_deadlock = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-config") == 0 && i + 1 < argc) o.config = argv[++i];
        else if (strcmp(argv[i], "-deadlock") == 0) o.check_deadlock = 0;
        else if (strcmp(argv[i], "-simulate") == 0) {
            o.simulate = 1;
            if (i + 1 < argc && (strstr(argv[i + 1], "num=") || strstr(argv[i + 1], "file="))) {
                for (char *arg = strtok(argv[++i], ","); arg; arg = strtok(NULL, ","))
                    if (strncmp(arg, "num=", 4) == 0) o.traces = atol(arg + 4);
                    else if (strncmp(arg, "file=", 5) == 0) o.trace_file = arg + 5;
                    else { fprintf(stderr, "tpp: -simulate %s is not supported\n", arg); return TPP_ERROR; }
            }
        }
        else if (strcmp(argv[i], "-depth") == 0 && i + 1 < argc) o.depth = atol(argv[++i]);
        else if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) { o.seed = atoll(argv[++i]); o.seed_given = 1; }
        else if (strcmp(argv[i], "-aril") == 0 && i + 1 < argc) o.aril = atoll(argv[++i]);
        else if (strcmp(argv[i], "-workers") == 0 && i + 1 < argc) {
            i++;
            o.workers = strcmp(argv[i], "auto") == 0 ? (int)sysconf(_SC_NPROCESSORS_ONLN) : atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-dump") == 0 && i + 2 < argc) {
            /* -dump dot[,actionlabels,...] FILE writes FILE.dot, as TLC. */
            const char *format = argv[++i], *file = argv[++i];
            if (strncmp(format, "dot", 3) != 0 || (format[3] && format[3] != ',')) {
                fprintf(stderr, "tpp: -dump writes only dot: -dump dot FILE\n");
                return TPP_ERROR;
            }
            o.dump_actions = strstr(format, "actionlabels") != NULL;
            size_t n = strlen(file);
            char *path = xmalloc(n + 5);
            sprintf(path, "%s%s", file, n >= 4 && strcmp(file + n - 4, ".dot") == 0 ? "" : ".dot");
            o.dump = path;
        } else if (strcmp(argv[i], "-version") == 0) { printf("tpp version %s\n", TPP_VERSION); return 0; }
        else if (strcmp(argv[i], "-nowarning") == 0) o.nowarning = 1;
        else if (strcmp(argv[i], "-difftrace") == 0) o.difftrace = 1;
        else if (strcmp(argv[i], "-continue") == 0) o.keep_going = 1;
        else if (strcmp(argv[i], "-view") == 0) o.view = 1;
        else if (strcmp(argv[i], "-userFile") == 0 && i + 1 < argc) o.user_file = argv[++i];
        /* TLC's, for its own files and fingerprints: tpp has neither */
        else if ((!strcmp(argv[i], "-metadir") || !strcmp(argv[i], "-fp") || !strcmp(argv[i], "-fpmem") || !strcmp(argv[i], "-fpbits")) && i + 1 < argc) i++;
        else if (!strcmp(argv[i], "-gzip") || !strcmp(argv[i], "-cleanup")) {}
        else if (argv[i][0] == '-') { fprintf(stderr, "tpp: unknown option %s\n", argv[i]); return TPP_ERROR; }
        else o.spec = argv[i];
    }
    if (!o.spec) {
        fprintf(stderr, "usage: tpp [-config FILE] [-deadlock] [-workers N|auto] [-dump dot FILE] SPEC\n");
        return TPP_ERROR;
    }
    return tpp_check(&o);
}
#endif
