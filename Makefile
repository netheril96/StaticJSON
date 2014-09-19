CXX ?= c++
CXXFLAGS ?= -std=c++11 -O2 -Wall -Wextra -pedantic -g

export

all:
	$(MAKE) -C test/

test:
	$(MAKE) test -C test/

clean:
	$(MAKE) clean -C test/

.PHONY: test clean
