/* tpp-std: the operators TLC computes in Java for its modules and for
   CommunityModules', written on tpp.h's native operators. Each is TLC's
   Java, error messages included (docs/std.md). */
#define _DEFAULT_SOURCE
#define _DARWIN_C_SOURCE
#include "tpp-std.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define J_V "tlc2.value.impl.Value"
#define J_EVAL "tlc2.tool.impl.Tool,tla2sany.semantic.ExprOrOpArgNode[],tlc2.util.Context," \
               "tlc2.tool.TLCState,tlc2.tool.TLCState,int,tlc2.tool.coverage.CostModel"

static TPPValue *str(const char *s) { return tpp_string(s, strlen(s)); }

/* A function whose keys are strings, as <<>> is: a record. */
static int is_record(TPPValue *v) {
    if (tpp_kind(v) != TPP_FUNCTION && tpp_kind(v) != TPP_TUPLE) return 0;
    for (size_t i = 0; i < tpp_size(v); i++) if (tpp_kind(tpp_key(v, i)) != TPP_STRING) return 0;
    return 1;
}

/* A growing array of values. */
typedef struct { TPPValue **v; size_t n, cap; } Values;

static void add(Values *l, TPPValue *v) {
    if (l->n == l->cap) {
        l->cap = l->cap ? 2 * l->cap : 8;
        l->v = realloc(l->v, sizeof(TPPValue *) * l->cap);
        if (!l->v) abort();
    }
    l->v[l->n++] = v;
}

/* ---------------------------------------------------------------- IOUtils */

extern char **environ;

static TPPValue *io_env(TPPCall *call, TPPValue **a) {
    (void)call, (void)a;
    Values names = {0}, values = {0};
    for (char **e = environ; *e; e++) {
        const char *eq = strchr(*e, '=');
        if (!eq) continue;
        add(&names, tpp_string(*e, (size_t)(eq - *e)));
        add(&values, str(eq + 1));
    }
    TPPValue *r = tpp_record(names.n, names.v, values.v);
    free(names.v), free(values.v);
    return r;
}

static TPPValue *atoi_(TPPCall *call, TPPValue **a) {
    if (tpp_kind(a[0]) == TPP_STRING) {   /* Java's Integer.parseInt */
        const char *t = tpp_string_value(a[0], NULL), *d = t + (*t == '-' || *t == '+');
        char *end;
        errno = 0;
        long long i = *d >= '0' && *d <= '9' ? strtoll(t, &end, 10) : 0;
        if (*d >= '0' && *d <= '9' && !*end && !errno && i >= INT32_MIN && i <= INT32_MAX) return tpp_int(i);
    }
    tpp_fail(call, "The argument of atoi should be a string, but instead it is:\n%s", tpp_print(a[0]));
}

/* The command's words, each a string, as IOExec takes them. */
static char **command_words(TPPCall *call, TPPValue *v, const char *op) {
    if (tpp_kind(v) != TPP_TUPLE) tpp_fail(call, "The argument of %s should be a sequence, but instead it is:\n%s", op, tpp_print(v));
    for (size_t i = 0; i < tpp_size(v); i++)
        if (tpp_kind(tpp_value(v, i)) != TPP_STRING)
            tpp_fail(call, "The argument of IOExec should be a sequence, but instead it is:\n%s", tpp_print(tpp_value(v, i)));
    char **argv = calloc(tpp_size(v) + 1, sizeof(char *));
    for (size_t i = 0; i < tpp_size(v); i++) argv[i] = (char *)tpp_string_value(tpp_value(v, i), NULL);
    return argv;
}

/* What IOExec and Serialize give: [exitValue, stdout, stderr]. */
static TPPValue *exec_record(int64_t code, const char *out, size_t nout, const char *err) {
    TPPValue *keys[] = {str("exitValue"), str("stdout"), str("stderr")};
    TPPValue *vals[] = {tpp_int(code), tpp_string(out, nout), str(err)};
    return tpp_record(3, keys, vals);
}

/* Runs argv with env's variables added, its standard input empty. */
static TPPValue *run_process(TPPCall *call, char **argv, TPPValue *env) {
    int out[2], err[2];
    fflush(stdout);
    if (pipe(out) || pipe(err)) tpp_fail(call, "Cannot run program \"%s\": %s", argv[0], strerror(errno));
    pid_t pid = fork();
    if (pid == 0) {
        int in = open("/dev/null", O_RDONLY);
        dup2(in, 0), dup2(out[1], 1), dup2(err[1], 2);
        close(out[0]), close(err[0]);
        for (size_t i = 0; env && i < tpp_size(env); i++) {
            TPPValue *v = tpp_value(env, i);
            setenv(tpp_string_value(tpp_key(env, i), NULL), tpp_kind(v) == TPP_STRING ? tpp_string_value(v, NULL) : tpp_print(v), 1);
        }
        execvp(argv[0], argv);
        _exit(127);
    }
    free(argv);
    close(out[1]), close(err[1]);
    char *text[2] = {NULL, NULL};
    size_t len[2] = {0, 0};
    struct pollfd fds[2] = {{out[0], POLLIN, 0}, {err[0], POLLIN, 0}};
    for (int open = 2; open;) {
        poll(fds, 2, -1);
        for (int i = 0; i < 2; i++) {
            if (fds[i].fd < 0 || !fds[i].revents) continue;
            char chunk[4096];
            ssize_t n = read(fds[i].fd, chunk, sizeof chunk);
            if (n <= 0) { close(fds[i].fd), fds[i].fd = -1, open--; continue; }
            text[i] = realloc(text[i], len[i] + (size_t)n + 1);
            memcpy(text[i] + len[i], chunk, (size_t)n);
            text[i][len[i] += (size_t)n] = 0;
        }
    }
    int status = 0;
    waitpid(pid, &status, 0);
    TPPValue *r = exec_record(WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status),
                              text[0] ? text[0] : "", len[0], text[1] ? text[1] : "");
    free(text[0]), free(text[1]);
    return r;
}

static TPPValue *io_exec(TPPCall *call, TPPValue **a) { return run_process(call, command_words(call, a[0], "IOExec"), NULL); }

static TPPValue *io_env_exec(TPPCall *call, TPPValue **a) {
    if (!is_record(a[0])) tpp_fail(call, "The argument of IOEnvExec should be a record, but instead it is:\n%s", tpp_print(a[0]));
    return run_process(call, command_words(call, a[1], "IOEnvExec"), a[0]);
}

/* Serialize(payload, file, [format |-> "TXT", charset |-> ..., openOptions |-> ...])
   writes the string payload to file, with Java's StandardOpenOptions; TLC
   leaves another format to the definition. */
static TPPValue *serialize(TPPCall *call, TPPValue **a) {
    (void)a;
    TPPValue *opts = tpp_arg(call, 2), *format = tpp_field(opts, "format");
    if (!format || tpp_kind(format) != TPP_STRING || strcmp(tpp_string_value(format, NULL), "TXT")) return NULL;
    TPPValue *payload = tpp_arg(call, 0), *file = tpp_arg(call, 1);
    TPPValue *options = tpp_field(opts, "openOptions"), *charset = tpp_field(opts, "charset");
    int ok = tpp_kind(payload) == TPP_STRING && tpp_kind(file) == TPP_STRING && options && tpp_kind(options) == TPP_TUPLE
          && charset && tpp_kind(charset) == TPP_STRING;
    int flags = ok && tpp_size(options) ? 0 : O_WRONLY | O_CREAT | O_TRUNC;
    static const struct { const char *name; int flags; } open_options[] = {
        {"READ", 0}, {"WRITE", O_WRONLY}, {"APPEND", O_WRONLY | O_APPEND}, {"TRUNCATE_EXISTING", O_TRUNC},
        {"CREATE", O_CREAT}, {"CREATE_NEW", O_CREAT | O_EXCL}, {"DSYNC", O_DSYNC}, {"SYNC", O_SYNC}, {NULL, 0}};
    for (size_t i = 0; ok && i < tpp_size(options); i++) {
        TPPValue *o = tpp_value(options, i);
        int k = 0;
        while (open_options[k].name && (tpp_kind(o) != TPP_STRING || strcmp(open_options[k].name, tpp_string_value(o, NULL)))) k++;
        ok = open_options[k].name != NULL;
        flags |= ok ? open_options[k].flags : 0;
    }
    if (!ok) return exec_record(1, "", 0, "Serialize error invalid parameters");
    size_t n;
    const char *text = tpp_string_value(payload, &n), *path = tpp_string_value(file, NULL);
    int fd = open(path, flags, 0644);
    int written = fd >= 0 && write(fd, text, n) == (ssize_t)n, err = errno;
    if (fd >= 0) close(fd);
    if (written) return exec_record(0, "Finish writing to the file with success!", 40, "");
    char message[4096];
    snprintf(message, sizeof message, "Serialize error writing to the file: %s: %s", path, strerror(err));
    return exec_record(1, "", 0, message);
}

/* ---------------------------------------------------------------- SequencesExt */

/* A finite set's elements in order. */
static TPPValue *set_to_seq(TPPCall *call, TPPValue **a) {
    TPPValue *s = tpp_elements(a[0]);
    if (!s) tpp_fail(call, "Attempted to enumerate a set that is not enumerable:\n%s", tpp_print(a[0]));
    Values l = {0};
    for (size_t i = 0; i < tpp_size(s); i++) add(&l, tpp_element(s, i));
    TPPValue *t = tpp_tuple(l.n, l.v);
    free(l.v);
    return t;
}

/* ---------------------------------------------------------------- SVG */

/* An element [name, attrs, children, innerText] as its markup. */
typedef struct { char *s; size_t n; } Text;

static void put(Text *t, const char *s, size_t n) {
    t->s = realloc(t->s, t->n + n + 1);
    memcpy(t->s + t->n, s, n);
    t->s[t->n += n] = 0;
}
static void puts_(Text *t, const char *s) { put(t, s, strlen(s)); }

static void svg_markup(TPPCall *call, Text *t, TPPValue *elem) {
    TPPValue *name = tpp_field(elem, "name"), *attrs = tpp_field(elem, "attrs");
    TPPValue *children = tpp_field(elem, "children"), *text = tpp_field(elem, "innerText");
    if (!name || tpp_kind(name) != TPP_STRING || !attrs || !children || !text || tpp_kind(text) != TPP_STRING
        || (tpp_kind(attrs) != TPP_FUNCTION && tpp_kind(attrs) != TPP_TUPLE)
        || (tpp_kind(children) != TPP_FUNCTION && tpp_kind(children) != TPP_TUPLE))
        tpp_fail(call, "An SVG element must be a record of name, attrs, children and innerText:\n%s", tpp_print(elem));
    puts_(t, "<"), puts_(t, tpp_string_value(name, NULL));
    for (size_t i = 0; tpp_kind(attrs) == TPP_FUNCTION && i < tpp_size(attrs); i++) {
        TPPValue *k = tpp_key(attrs, i), *v = tpp_value(attrs, i);
        if (tpp_kind(k) != TPP_STRING || tpp_kind(v) != TPP_STRING) tpp_fail(call, "An SVG attribute must be a string:\n%s", tpp_print(attrs));
        puts_(t, " ");
        for (const char *c = tpp_string_value(k, NULL); *c; c++) put(t, *c == '_' ? "-" : c, 1);
        puts_(t, "='"), puts_(t, tpp_string_value(v, NULL)), puts_(t, "'");
    }
    puts_(t, ">");
    for (const char *c = tpp_string_value(text, NULL); *c; c++) {   /* a tuple <<1, 2>> in the text */
        if (!strncmp(c, "<<", 2)) puts_(t, "&lt;&lt;"), c++;
        else if (!strncmp(c, ">>", 2)) puts_(t, "&gt;&gt;"), c++;
        else put(t, c, 1);
    }
    for (size_t i = 0; i < tpp_size(children); i++) svg_markup(call, t, tpp_value(children, i));
    puts_(t, "</"), puts_(t, tpp_string_value(name, NULL)), puts_(t, ">");
}

static TPPValue *svg_elem_to_string(TPPCall *call, TPPValue **a) {
    Text t = {0};
    svg_markup(call, &t, a[0]);
    TPPValue *v = tpp_string(t.s ? t.s : "", t.n);
    free(t.s);
    return v;
}

/* ---------------------------------------------------------------- Json */

/* JSON text as Json's getValue makes it a value: an array a tuple, an
   object a record, a number Gson's getAsInt, true, false and strings
   themselves; null is no value. */
typedef struct { TPPCall *call; const char *p, *end; } Json;

static void json_space(Json *j) { while (j->p < j->end && strchr(" \t\r\n", *j->p)) j->p++; }

static _Noreturn void json_bad(Json *j) {
    tpp_throw(j->call, "com.google.gson.JsonSyntaxException: malformed JSON at %.20s", j->p < j->end ? j->p : "end of input");
}

static void json_expect(Json *j, char c) {
    json_space(j);
    if (j->p == j->end || *j->p != c) json_bad(j);
    j->p++;
}

/* The UTF-8 of code point u, at t. */
static void utf8(Text *t, unsigned u) {
    char b[4];
    size_t n = u < 0x80 ? 1 : u < 0x800 ? 2 : u < 0x10000 ? 3 : 4;
    if (n == 1) b[0] = (char)u;
    else {
        for (size_t i = n - 1; i > 0; i--) b[i] = (char)(0x80 | (u & 0x3f)), u >>= 6;
        b[0] = (char)(((0xf00 >> n) & 0xff) | u);
    }
    put(t, b, n);
}

static unsigned json_hex4(Json *j) {
    unsigned u = 0;
    for (int i = 0; i < 4; i++, j->p++) {
        char c = j->p < j->end ? *j->p : 0;
        int d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
        if (d < 0) json_bad(j);
        u = u * 16 + (unsigned)d;
    }
    return u;
}

static TPPValue *json_string(Json *j) {
    json_expect(j, '"');
    Text t = {0};
    put(&t, "", 0);
    while (j->p < j->end && *j->p != '"') {
        if (*j->p != '\\') { put(&t, j->p++, 1); continue; }
        if (++j->p == j->end) json_bad(j);
        char c = *j->p++;
        switch (c) {
        case '"': case '\\': case '/': put(&t, &c, 1); break;
        case 'b': put(&t, "\b", 1); break;
        case 'f': put(&t, "\f", 1); break;
        case 'n': put(&t, "\n", 1); break;
        case 'r': put(&t, "\r", 1); break;
        case 't': put(&t, "\t", 1); break;
        case 'u': {
            unsigned u = json_hex4(j);
            if (u >= 0xd800 && u < 0xdc00 && j->end - j->p >= 6 && j->p[0] == '\\' && j->p[1] == 'u') {   /* a surrogate pair */
                j->p += 2;
                u = 0x10000 + ((u - 0xd800) << 10) + (json_hex4(j) - 0xdc00);
            }
            utf8(&t, u);
            break;
        }
        default: json_bad(j);
        }
    }
    if (j->p == j->end) json_bad(j);
    j->p++;
    TPPValue *v = tpp_string(t.s, t.n);
    free(t.s);
    return v;
}

static TPPValue *json_value(Json *j) {
    json_space(j);
    if (j->p == j->end) json_bad(j);
    if (*j->p == '"') return json_string(j);
    if (*j->p == '[' || *j->p == '{') {
        int object = *j->p++ == '{';
        Values keys = {0}, values = {0};
        json_space(j);
        if (j->p < j->end && *j->p == (object ? '}' : ']')) j->p++;
        else for (;;) {
            if (object) {   /* Gson keeps a key's last value */
                TPPValue *k = json_string(j);
                json_expect(j, ':');
                size_t i = 0;
                while (i < keys.n && tpp_compare(keys.v[i], k)) i++;
                if (i == keys.n) add(&keys, k), add(&values, NULL);
                values.v[i] = json_value(j);
            } else {
                add(&values, json_value(j));
            }
            json_space(j);
            if (j->p < j->end && *j->p == ',') { j->p++; continue; }
            json_expect(j, object ? '}' : ']');
            break;
        }
        TPPValue *v = object ? tpp_record(keys.n, keys.v, values.v) : tpp_tuple(values.n, values.v);
        free(keys.v), free(values.v);
        return v;
    }
    const char *word = j->p;
    while (j->p < j->end && !strchr(" \t\r\n,]}", *j->p)) j->p++;
    size_t n = (size_t)(j->p - word);
    if (n == 4 && !strncmp(word, "true", 4)) return tpp_bool(1);
    if (n == 5 && !strncmp(word, "false", 5)) return tpp_bool(0);
    if (n == 4 && !strncmp(word, "null", 4))
        tpp_throw(j->call, "java.io.IOException: Cannot convert value: unsupported JSON value null");
    char number[64], *end;
    if (n == 0 || n >= sizeof number) json_bad(j);
    memcpy(number, word, n), number[n] = 0;
    errno = 0;
    long long i = strtoll(number, &end, 10);   /* getAsInt: an int's low bits, or a decimal's truncated */
    if (*end || errno) {
        double d = strtod(number, &end);
        if (*end) { j->p = word; json_bad(j); }
        i = d >= 9.2e18 || d <= -9.2e18 ? 0 : (long long)d;
    }
    return tpp_int((int32_t)(uint32_t)(unsigned long long)i);
}

/* The file at path, or TLC's error that it cannot be read. */
static char *json_file(TPPCall *call, TPPValue *path, size_t *n) {
    if (tpp_kind(path) != TPP_STRING) tpp_fail(call, "The argument should be a string, but instead it is:\n%s", tpp_print(path));
    const char *name = tpp_string_value(path, NULL);
    FILE *f = fopen(name, "rb");
    if (!f) tpp_throw(call, "%s (%s)", name, strerror(errno));
    Text t = {0};
    put(&t, "", 0);
    char chunk[1 << 16];
    for (size_t got; (got = fread(chunk, 1, sizeof chunk, f)) > 0;) put(&t, chunk, got);
    fclose(f);
    *n = t.n;
    return t.s;
}

static TPPValue *json_deserialize(TPPCall *call, TPPValue **a) {
    size_t n;
    char *text = json_file(call, a[0], &n);
    Json j = {call, text, text + n};
    TPPValue *v = json_value(&j);
    json_space(&j);
    if (j.p != j.end) json_bad(&j);
    free(text);
    return v;
}

/* Each line not blank a value: ndJSON. */
static TPPValue *nd_json_deserialize(TPPCall *call, TPPValue **a) {
    size_t n;
    char *text = json_file(call, a[0], &n);
    Values lines = {0};
    for (const char *p = text, *end = text + n; p < end;) {
        const char *eol = memchr(p, '\n', (size_t)(end - p));
        Json j = {call, p, eol ? eol : end};
        json_space(&j);
        if (j.p < j.end) {
            add(&lines, json_value(&j));
            json_space(&j);
            if (j.p != j.end) json_bad(&j);
        }
        p = eol ? eol + 1 : end;
    }
    TPPValue *v = tpp_tuple(lines.n, lines.v);
    free(lines.v), free(text);
    return v;
}

/* ---------------------------------------------------------------- Graphs, UndirectedGraphs */

static TPPValue *graph_set(TPPValue *g, const char *field) {
    TPPValue *v = tpp_field(g, field);
    return v ? tpp_elements(v) : NULL;
}

/* G's nodes and edges, enumerated, if G is a graph record; else the error
   TLC gives for argument POS of OP, a KIND. */
static void to_graph(TPPCall *call, const char *op, const char *pos, const char *kind, TPPValue *g,
                     TPPValue **nodes, TPPValue **edges) {
    const char *bad = !is_record(g) ? "record with a node and an edge field"
                    : !(*nodes = graph_set(g, "node")) ? "record whose node field is a set"
                    : !(*edges = graph_set(g, "edge")) ? "record whose edge field is a set" : NULL;
    if (bad) tpp_fail(call, "The %s argument of %s should be a %s %s, but instead it is:\n%s", pos, op, kind, bad, tpp_print(g));
}

static long position(TPPValue *set, TPPValue *v) {
    for (long lo = 0, hi = (long)tpp_size(set) - 1; lo <= hi;) {
        long m = (lo + hi) / 2;
        int c = tpp_compare(tpp_element(set, (size_t)m), v);
        if (!c) return m;
        if (c < 0) lo = m + 1; else hi = m - 1;
    }
    return -1;
}

static size_t root(size_t *parent, size_t i) {
    while (parent[i] != i) i = parent[i] = parent[parent[i]];
    return i;
}

/* The components as TLC's union-find finds them: an edge is a set of one
   or two nodes, and other edges, or those leaving the nodes, join nothing. */
static TPPValue *connected_components(TPPCall *call, TPPValue **a) {
    TPPValue *nodes, *edges;
    to_graph(call, "ConnectedComponents", "first", "undirected graph", a[0], &nodes, &edges);
    size_t n = tpp_size(nodes), *parent = malloc(sizeof(size_t) * (n + 1));
    for (size_t i = 0; i < n; i++) parent[i] = i;
    for (size_t k = 0; k < tpp_size(edges); k++) {
        TPPValue *e = tpp_elements(tpp_element(edges, k));
        if (!e || tpp_size(e) == 0 || tpp_size(e) > 2) continue;
        long i = position(nodes, tpp_element(e, 0)), j = position(nodes, tpp_element(e, tpp_size(e) - 1));
        if (i >= 0 && j >= 0) parent[root(parent, (size_t)j)] = root(parent, (size_t)i);
    }
    Values *members = calloc(n + 1, sizeof(Values)), out = {0};
    for (size_t i = 0; i < n; i++) add(&members[root(parent, i)], tpp_element(nodes, i));
    for (size_t i = 0; i < n; i++)
        if (members[i].n) add(&out, tpp_set(members[i].n, members[i].v)), free(members[i].v);
    TPPValue *r = tpp_set(out.n, out.v);
    free(out.v), free(members), free(parent);
    return r;
}

/* A directed graph's arcs, by the positions of its nodes: those of node i
   go to to[start[i]] .. to[start[i + 1] - 1]. An edge is a pair of nodes;
   another, or one leaving the nodes, is none, as in Graphs' definitions. */
typedef struct { size_t n, *start, *to; } Arcs;

static Arcs arcs_of(TPPValue *nodes, TPPValue *edges, int transpose) {
    size_t n = tpp_size(nodes), m = tpp_size(edges), k = 0;
    long *from = malloc(sizeof(long) * (m + 1)), *to = malloc(sizeof(long) * (m + 1));
    Arcs a = {n, calloc(n + 2, sizeof(size_t)), malloc(sizeof(size_t) * (m + 1))};
    for (size_t i = 0; i < m; i++) {
        TPPValue *e = tpp_element(edges, i);
        if (tpp_kind(e) != TPP_TUPLE || tpp_size(e) != 2) continue;
        from[k] = position(nodes, tpp_value(e, transpose)), to[k] = position(nodes, tpp_value(e, !transpose));
        if (from[k] >= 0 && to[k] >= 0) a.start[from[k] + 2]++, k++;
    }
    for (size_t i = 2; i <= n + 1; i++) a.start[i] += a.start[i - 1];
    for (size_t i = 0; i < k; i++) a.to[a.start[from[i] + 1]++] = (size_t)to[i];   /* start[f + 1]: f's next arc */
    free(from), free(to);
    return a;
}

static void arcs_free(Arcs a) { free(a.start), free(a.to); }

/* How many nodes are reachable from node s, each marked in seen. */
static size_t reach(Arcs a, size_t s, char *seen) {
    size_t *queue = malloc(sizeof(size_t) * (a.n + 1)), head = 0, tail = 0;
    seen[s] = 1, queue[tail++] = s;
    while (head < tail)
        for (size_t i = queue[head++], k = a.start[i]; k < a.start[i + 1]; k++)
            if (!seen[a.to[k]]) seen[a.to[k]] = 1, queue[tail++] = a.to[k];
    free(queue);
    return tail;
}

static TPPValue *are_connected_in(TPPCall *call, TPPValue **a) {
    TPPValue *nodes, *edges;
    to_graph(call, "AreConnectedIn", "third", "graph", a[2], &nodes, &edges);
    long m = position(nodes, a[0]), n = position(nodes, a[1]);
    if (m < 0 || n < 0) return tpp_bool(0);
    if (m == n) return tpp_bool(1);
    Arcs arcs = arcs_of(nodes, edges, 0);
    char *seen = calloc(arcs.n, 1);
    reach(arcs, (size_t)m, seen);
    int r = seen[n];
    free(seen), arcs_free(arcs);
    return tpp_bool(r);
}

/* Every node reaches the first, and the first every node. */
static TPPValue *is_strongly_connected(TPPCall *call, TPPValue **a) {
    TPPValue *nodes, *edges;
    to_graph(call, "IsStronglyConnected", "first", "graph", a[0], &nodes, &edges);
    int r = 1;
    for (int transpose = 0; transpose < 2 && r && tpp_size(nodes); transpose++) {
        Arcs arcs = arcs_of(nodes, edges, transpose);
        char *seen = calloc(arcs.n, 1);
        r = reach(arcs, 0, seen) == arcs.n;
        free(seen), arcs_free(arcs);
    }
    return tpp_bool(r);
}

/* Each simple path that path, of n nodes, starts. */
static void simple_paths(Arcs a, size_t *path, size_t n, char *on, TPPValue *nodes, Values *out) {
    TPPValue **elems = malloc(sizeof(TPPValue *) * n);
    for (size_t i = 0; i < n; i++) elems[i] = tpp_element(nodes, path[i]);
    add(out, tpp_tuple(n, elems));
    free(elems);
    for (size_t k = a.start[path[n - 1]]; k < a.start[path[n - 1] + 1]; k++)
        if (!on[a.to[k]]) {
            on[path[n] = a.to[k]] = 1;
            simple_paths(a, path, n + 1, on, nodes, out);
            on[a.to[k]] = 0;
        }
}

static TPPValue *simple_path(TPPCall *call, TPPValue **a) {
    TPPValue *nodes, *edges;
    to_graph(call, "SimplePath", "first", "graph", a[0], &nodes, &edges);
    Arcs arcs = arcs_of(nodes, edges, 0);
    size_t *path = malloc(sizeof(size_t) * (arcs.n + 1));
    char *on = calloc(arcs.n + 1, 1);
    Values out = {0};
    for (size_t s = 0; s < arcs.n; s++) on[path[0] = s] = 1, simple_paths(arcs, path, 1, on, nodes, &out), on[s] = 0;
    TPPValue *r = tpp_set(out.n, out.v);
    free(out.v), free(on), free(path), arcs_free(arcs);
    return r;
}

/* ---------------------------------------------------------------- the table */

#define OVERRIDE(module, method) "public static " J_V " tlc2.overrides." module "." method

int tpp_std_init(void) {
    static const struct { const char *module, *name; int arity; TPPNative fn; const char *java; } natives[] = {
        {"IOUtils", "IOEnv", 0, io_env, OVERRIDE("IOUtils", "ioEnv()")},
        {"IOUtils", "atoi", 1, atoi_, OVERRIDE("IOUtils", "atoi(" J_V ")")},
        {"IOUtils", "IOExec", 1, io_exec, OVERRIDE("IOUtils", "ioExec(" J_V ")")},
        {"IOUtils", "IOEnvExec", 2, io_env_exec, OVERRIDE("IOUtils", "ioEnvExec(" J_V "," J_V ")")},
        {"IOUtils", "Serialize", TPP_LAZY, serialize,
         "public static synchronized " J_V " tlc2.overrides.IOUtils.textSerialize(" J_EVAL ")"},
        {"SequencesExt", "SetToSeq", 1, set_to_seq, OVERRIDE("SequencesExt", "SetToSeq(" J_V ")")},
        {"SVG", "SVGElemToString", 1, svg_elem_to_string, OVERRIDE("SVG", "SVGElemToString(" J_V ")")},
        {"Json", "JsonDeserialize", 1, json_deserialize,
         "public static tlc2.value.IValue tlc2.overrides.Json.deserialize(tlc2.value.impl.StringValue) throws java.io.IOException"},
        {"Json", "ndJsonDeserialize", 1, nd_json_deserialize,
         "public static tlc2.value.IValue tlc2.overrides.Json.ndDeserialize(tlc2.value.impl.StringValue) throws java.io.IOException"},
        {"Graphs", "SimplePath", 1, simple_path, OVERRIDE("Graphs", "simplePath(" J_V ")")},
        {"Graphs", "AreConnectedIn", 3, are_connected_in, OVERRIDE("Graphs", "areConnectedIn(" J_V "," J_V "," J_V ")")},
        {"Graphs", "IsStronglyConnected", 1, is_strongly_connected, OVERRIDE("Graphs", "isStronglyConnected(" J_V ")")},
        {"UndirectedGraphs", "ConnectedComponents", 1, connected_components,
         OVERRIDE("UndirectedGraphs", "connectedComponents(" J_V ")")},
    };
    for (size_t i = 0; i < sizeof natives / sizeof *natives; i++)
        if (tpp_define(natives[i].module, natives[i].name, natives[i].arity, natives[i].fn, natives[i].java)) return -1;
    return 0;
}
