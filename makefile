export ROOT := $(shell pwd)
export SRCDIR := $(ROOT)/src
export INNEROBJS := $(SRCDIR)/om_internal.o $(SRCDIR)/om_api.o $(SRCDIR)/om_type.o $(SRCDIR)/om_hash.o

all: 
	$(MAKE) -C src all

package:
	$(MAKE) -C src package

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
	$(RM) -f print_version

test: 
	$(MAKE) -C src all
	$(MAKE) -C tests all

perf: 
	$(MAKE) -C src all
	$(MAKE) -C tests perf

docker:
	# TODO