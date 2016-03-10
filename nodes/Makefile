SUBDIRS = Common Dimmer WC Valve Player Bomb PBX P2K Map Key Floor RFID
SUBCLEAN = $(addsuffix .clean, $(SUBDIRS))

.PHONY: all $(SUBDIRS) clean $(SUBCLEAN)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean: $(SUBCLEAN)

$(SUBCLEAN): %.clean:
	$(MAKE) -C $* clean

