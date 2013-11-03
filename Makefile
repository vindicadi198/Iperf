.PHONY: defaults

defaults: all

%:
	$(MAKE) -C src $@
