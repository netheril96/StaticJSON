all:
	$(MAKE) -C test/

test:
	$(MAKE) test -C test/

clean:
	$(MAKE) clean -C test/

.PHONY: test clean
