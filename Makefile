CC = cc
CFLAGS = -O2 -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -lm
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

# TLA+ tools 1.7.4 (TLC 2.19), the oracle, pinned by checksum.
TLA_JAR = $(OBJDIR)/tla/tla2tools.jar
TLA_URL = https://github.com/tlaplus/tlaplus/releases/download/v1.7.4/tla2tools.jar
TLA_SHA256 = 936a262061c914694dfd669a543be24573c45d5aa0ff20a8b96b23d01e050e88
JAVA ?= $(firstword $(wildcard /opt/homebrew/opt/openjdk/bin/java) java)
TLC = $(JAVA) -XX:+UseParallelGC -cp $(abspath $(TLA_JAR)) tlc2.TLC

.PHONY: all release test test-spec test-spec-oracle bless test-tla tla-tools sanitize fuzz fuzz-oracle bench install clean

all: tpp

tpp: tpp.c tpp.h
	$(CC) $(CFLAGS) $(LDFLAGS) tpp.c $(LDLIBS) -o $@

# tpp with profile-guided optimization: the conformance suite is the
# training run. Clang merges its raw profiles with llvm-profdata.
PGO = $(abspath $(OBJDIR))/pgo
ifneq ($(findstring clang,$(shell $(CC) --version 2>/dev/null)),)
PROFDATA ?= $(shell xcrun -f llvm-profdata 2>/dev/null || command -v llvm-profdata)
PGO_GEN = -fprofile-instr-generate
PGO_USE = -fprofile-instr-use=$(PGO)/tpp.profdata
PGO_MERGE = $(PROFDATA) merge -o $(PGO)/tpp.profdata $(PGO)/*.profraw
else
PGO_GEN = -fprofile-generate=$(PGO)
PGO_USE = -fprofile-use=$(PGO) -fprofile-partial-training
PGO_MERGE = true
endif

release: tpp.c tpp.h
	rm -rf $(PGO) && mkdir -p $(PGO)
	$(CC) $(CFLAGS) $(PGO_GEN) -c tpp.c -o $(PGO)/tpp.o
	$(CC) $(PGO_GEN) $(LDFLAGS) $(PGO)/tpp.o $(LDLIBS) -o $(PGO)/tpp
	LLVM_PROFILE_FILE=$(PGO)/%p.profraw python3 tests/spec_runner.py $(PGO)/tpp > /dev/null
	$(PGO_MERGE)
	$(CC) $(CFLAGS) $(PGO_USE) -c tpp.c -o $(PGO)/tpp.o
	$(CC) $(LDFLAGS) $(PGO)/tpp.o $(LDLIBS) -o tpp

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
# must print as TLC prints them. RUNS and SEED choose how many and which.
RUNS ?= 1000
SEED ?= 1
fuzz: $(OBJDIR)/tpp-sanitize
	$(SANITIZE_ENV) python3 tests/fuzz.py crash $(OBJDIR)/tpp-sanitize --runs $(RUNS) --seed $(SEED)

fuzz-oracle: tpp $(TLA_JAR)
	python3 tests/fuzz.py diff ./tpp --tlc "$(TLC)" --runs $(RUNS) --seed $(SEED)

bench: release $(TLA_JAR)
	python3 bench/compare.py ./tpp --tlc "$(TLC)"

install: tpp
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/include
	install tpp $(DESTDIR)$(PREFIX)/bin/tpp
	install -m 644 tpp.h $(DESTDIR)$(PREFIX)/include/tpp.h

clean:
	rm -f tpp
