DEBUGFLAGS = -g -ggdb -O2
ifeq ($(DEBUG), 1)
	DEBUGFLAGS = -g -ggdb -O0
endif

TESTDIR := $(ROOT)/tests

# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
CFLAGS =  -Wall $(DEBUGFLAGS) -fPIC -std=gnu99 -D_GNU_SOURCE -I$(SRCDIR) -I$(TESTDIR)
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')

# Compile flags for linux / osx
ifneq ($(uname_S),Linux)
	CFLAGS += -mmacosx-version-min=10.6
endif

ROOT=$(shell pwd)
LDFLAGS = -lm -lc

all: inner api

inner: test_inner
	./test_inner

api:
	python test_api.py

perf: test_perf
	./test_perf

test_inner: test_inner.o $(INNEROBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_perf: test_perf.o $(INNEROBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) test_inner test_perf *.o *.rdb *.pyc