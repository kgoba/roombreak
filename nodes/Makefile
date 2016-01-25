SUBDIRS = Common Bomb PBX
SUBCLEAN = $(addsuffix .clean, $(SUBDIRS))

.PHONY: all $(SUBDIRS) clean $(SUBCLEAN)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean: $(SUBCLEAN)

$(SUBCLEAN): %.clean:
	$(MAKE) -C $* clean

