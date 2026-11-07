CC = cc
CFLAGS = -O2 -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -lm
# Cygwin's main thread has the stack its executable reserves, less than the
# 8 MB elsewhere: evaluation nests 5000 deep before TLC's StackOverflowError.
ifneq ($(findstring CYGWIN,$(shell uname)),)
LDLIBS += -Wl,--stack,67108864
endif
# AddressSanitizer hangs at startup on recent macOS; there, Guard Malloc
# catches overruns and use-after-free instead.
ifeq ($(shell uname),Darwin)
SANITIZERS = undefined
SANITIZE_ENV = DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib
else
SANITIZERS = address,undefined
endif
PREFIX ?= /usr/local
OBJDIR = .obj

# TLA+ tools 1.8.0, the oracle, pinned by checksum. 1.8.0 is a pre-release,
# whose jar can be rebuilt under the same tag: this one is commit d8fb231de4e4
# (its manifest's X-Git-Revision) and prints Version 2026.11.06.222357.
TLA_JAR = $(OBJDIR)/tla/tla2tools.jar
TLA_URL = https://github.com/tlaplus/tlaplus/releases/download/v1.8.0/tla2tools.jar
TLA_SHA256 = e1f11b75c4b5bc4478ba0ac5cb6ae28e40a01b0016bc33e0aa97da22dfdbcd7f
JAVA ?= $(firstword $(wildcard /opt/homebrew/opt/openjdk/bin/java) java)
TLC = $(JAVA) -XX:+UseParallelGC -cp $(abspath $(TLA_JAR)) tlc2.TLC

.PHONY: all test test-spec test-spec-oracle bless test-tla tla-tools sanitize fuzz fuzz-oracle fuzz-sim bench install clean

all: tpp

# ./tpp is profile-guided: the conformance suite is the training run.
# Without python3 to run it (or, with clang, llvm-profdata to merge the
# profile), ./tpp is a plain build.
PGO = $(abspath $(OBJDIR))/pgo
PYTHON = $(shell command -v python3)
ifneq ($(findstring clang,$(shell $(CC) --version 2>/dev/null)),)
PROFDATA ?= $(shell xcrun -f llvm-profdata 2>/dev/null || command -v llvm-profdata)
PGO_TOOLS = $(and $(PYTHON),$(PROFDATA))
PGO_GEN = -fprofile-instr-generate
PGO_USE = -fprofile-instr-use=$(PGO)/tpp.profdata
PGO_MERGE = $(PROFDATA) merge -o $(PGO)/tpp.profdata $(PGO)/*.profraw
else
PGO_TOOLS = $(PYTHON)
PGO_GEN = -fprofile-generate=$(PGO)
PGO_USE = -fprofile-use=$(PGO) -fprofile-partial-training
PGO_MERGE = true
endif

tpp: tpp.c tpp.h
ifeq ($(PGO_TOOLS),)
	$(CC) $(CFLAGS) $(LDFLAGS) tpp.c $(LDLIBS) -o $@
else
	rm -rf $(PGO) && mkdir -p $(PGO)
	$(CC) $(CFLAGS) $(PGO_GEN) -c tpp.c -o $(PGO)/tpp.o
	$(CC) $(PGO_GEN) $(LDFLAGS) $(PGO)/tpp.o $(LDLIBS) -o $(PGO)/tpp
	LLVM_PROFILE_FILE=$(PGO)/%p.profraw $(PYTHON) tests/spec_runner.py $(PGO)/tpp > /dev/null
	$(PGO_MERGE)
	$(CC) $(CFLAGS) $(PGO_USE) -c tpp.c -o $(PGO)/tpp.o
	$(CC) $(LDFLAGS) $(PGO)/tpp.o $(LDLIBS) -o $@
endif

test: test-spec

test-spec: tpp
	python3 tests/spec_runner.py ./tpp --strict
	python3 tests/spec_runner.py "./tpp -workers 4" --strict

# The oracle must agree with every expected output, which validates the tests.
test-spec-oracle: tpp $(TLA_JAR)
	python3 tests/spec_runner.py ./tpp --strict --oracle "$(TLC)"

# Rewrite the expected outputs from the oracle (after adding a test).
bless: $(TLA_JAR)
	python3 tests/spec_runner.py ./tpp --oracle "$(TLC)" --bless $(if $(ONLY),--only $(ONLY))

test-tla: tpp $(TLA_JAR)
	python3 tests/tla_check.py ./tpp --tlc "$(TLC)"

tla-tools: $(TLA_JAR)
$(TLA_JAR):
	mkdir -p $(dir $@)
	curl -fsSL -o $@.tmp $(TLA_URL)
	echo "$(TLA_SHA256)  $@.tmp" | shasum -a 256 -c -
	mv $@.tmp $@

$(OBJDIR)/tpp-sanitize: tpp.c tpp.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -O1 -g -fsanitize=$(SANITIZERS) -fno-sanitize-recover=all tpp.c $(LDLIBS) -o $@

sanitize: $(OBJDIR)/tpp-sanitize
	$(SANITIZE_ENV) python3 tests/spec_runner.py $(OBJDIR)/tpp-sanitize --strict

# Mutated models must not crash the sanitized build; random expressions
# must print as TLC prints them; so must random simulations of the models.
# RUNS and SEED choose how many and which.
RUNS ?= 1000
SEED ?= 1
fuzz: $(OBJDIR)/tpp-sanitize
	$(SANITIZE_ENV) python3 tests/fuzz.py crash $(OBJDIR)/tpp-sanitize --runs $(RUNS) --seed $(SEED)

fuzz-oracle: tpp $(TLA_JAR)
	python3 tests/fuzz.py diff ./tpp --tlc "$(TLC)" --runs $(RUNS) --seed $(SEED)

fuzz-sim: tpp $(TLA_JAR)
	python3 tests/fuzz.py sim ./tpp --tlc "$(TLC)" --runs $(RUNS) --seed $(SEED)

bench: tpp $(TLA_JAR)
	python3 bench/compare.py ./tpp --tlc "$(TLC)"

install: tpp
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/include
	install tpp $(DESTDIR)$(PREFIX)/bin/tpp
	install -m 644 tpp.h $(DESTDIR)$(PREFIX)/include/tpp.h

clean:
	rm -f tpp
