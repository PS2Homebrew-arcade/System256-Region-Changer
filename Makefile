# [ASIA5, ASIA4, JAPAN]
REGION ?= JAPAN
IOP_MODULES ?= $(addprefix irx/, $(addsuffix .irx, fileXio iomanX))

EE_BIN = bin/SYSTEM256_REGIONCHANGE_$(REGION).ELF
EE_OBJS = main.o
EE_LIBS += $(addprefix -l, fileXio cdvd debug patches)
EE_CFLAGS += -fdata-sections -ffunction-sections -DNEWLIB_PORT_AWARE
EE_LDFLAGS += -Wl,--gc-sections

ifeq ($(DEBUG), 1)
  EE_CFLAGS += -DDEBUG -O0 -g
else 
  EE_CFLAGS += -Os
  EE_LDFLAGS += -s
endif

ifeq ($(REGION), ASIA4)
  EE_CFLAGS += -DREGION=0
else ifeq ($(REGION), ASIA5)
  EE_CFLAGS += -DREGION=1
else ifeq ($(REGION), JAPAN)
  EE_CFLAGS += -DREGION=2
else
$(error invalid REGION index $(REGION))
endif

all: $(IOP_MODULES) $(EE_BIN)

clean:
	rm -rf $(EE_OBJS) $(EE_BIN)


vpath %.irx $(PS2SDK)/iop/irx/
irx/%.irx: %.irx
	$(DIR_GUARD)
	cp $< $@

# Include makefiles
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal