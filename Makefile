CC = cc
CFLAGS = -O2 -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -lm
SANITIZERS = address,undefined
PREFIX ?= /usr/local
OBJDIR = .obj

# TLA+ tools 1.7.4 (TLC 2.19), the oracle, pinned by checksum.
TLA_JAR = $(OBJDIR)/tla/tla2tools.jar
TLA_URL = https://github.com/tlaplus/tlaplus/releases/download/v1.7.4/tla2tools.jar
TLA_SHA256 = 936a262061c914694dfd669a543be24573c45d5aa0ff20a8b96b23d01e050e88
JAVA ?= $(firstword $(wildcard /opt/homebrew/opt/openjdk/bin/java) java)
TLC = $(JAVA) -XX:+UseParallelGC -cp $(abspath $(TLA_JAR)) tlc2.TLC

.PHONY: all test test-spec test-spec-oracle bless test-tla tla-tools sanitize bench install clean

all: tpp

tpp: tpp.c tpp.h
	$(CC) $(CFLAGS) $(LDFLAGS) tpp.c $(LDLIBS) -o $@

test: test-spec

test-spec: tpp
	python3 tests/spec_runner.py ./tpp --strict

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

sanitize:
	$(CC) $(CFLAGS) -O1 -g -fsanitize=$(SANITIZERS) tpp.c $(LDLIBS) -o $(OBJDIR)/tpp-sanitize
	python3 tests/spec_runner.py $(OBJDIR)/tpp-sanitize --strict

bench: tpp $(TLA_JAR)
	python3 bench/compare.py ./tpp --tlc "$(TLC)"

install: tpp
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/include
	install tpp $(DESTDIR)$(PREFIX)/bin/tpp
	install -m 644 tpp.h $(DESTDIR)$(PREFIX)/include/tpp.h

clean:
	rm -f tpp
