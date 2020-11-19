PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = diffdasm.o intstack.o memorymap.o memoryfile.o stats6809.o statsOS9.o

ifeq ($(BUILD_MODE),debug)
	CFLAGS += -g
else ifeq ($(BUILD_MODE),run)
	CFLAGS += -O2
else
#	$(error Build mode $(BUILD_MODE) not supported by this Makefile)
endif

all:	diffdasm

diffdasm:	$(OBJS)
	$(CXX) -o $@ $^

%.o:	$(PROJECT_ROOT)%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr build/make.debug.macosx.x86_64/diffdasm $(OBJS)

install:    all
	rm -f /usr/local/bin/diffdasm
	cp build/make.debug.macosx.x86_64/diffdasm /usr/local/bin/

