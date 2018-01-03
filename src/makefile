DEBUGFLAGS = -g -ggdb -O2
ifeq ($(DEBUG), 1)
	DEBUGFLAGS = -g -ggdb -O0
endif

# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
CFLAGS =  -Wall $(DEBUGFLAGS) -fPIC -std=gnu99 -D_GNU_SOURCE -I$(SRCDIR)
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')

OBJS = $(INNEROBJS) om_module.o print_version.o

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_CFLAGS ?=  -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -shared -Bsymbolic -Bsymbolic-functions
else
	CFLAGS += -mmacosx-version-min=10.6
	SHOBJ_CFLAGS ?= -dynamic -fno-common -g -ggdb
	SHOBJ_LDFLAGS ?= -dylib -exported_symbol _RedisModule_OnLoad -macosx_version_min 10.6
endif

LDFLAGS = -lm -lc
MODULE_SO = orderedset.so
CFLAGS += -DHASH_FUNCTION=HASH_MUR -DHASH_USING_NO_STRICT_ALIASING -fno-strict-aliasing

all: $(MODULE_SO)

$(MODULE_SO): $(OBJS)
	$(LD) $^ -o $@ $(SHOBJ_LDFLAGS) $(LDFLAGS)

clean:
	$(RM) -rv *.so *.o

# Compile an executable that prints the current version
print_version:  version.h print_version.c
	@$(CC) -o $@ -DPRINT_VERSION_TARGET $@.c