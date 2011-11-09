TARGET=pinkos

# hardcoded radio network address
DEVADDR = 0

# size of console line buffer
LINEBUF_MAX=32

APPLICATION_OFFSET=0x4000

#SVNREV = `svn info | grep "Last Changed Rev" | sed -e "s/.*: //"`
SVNREV = 0

CC = sdcc 
COMMON_CFLAGS = -Iinclude --no-pack-iram -DAPPLICATION_OFFSET=$(APPLICATION_OFFSET)
COMMON_CFLAGS += -DBUILD_VERSION=$(SVNREV) -DDEVADDR=$(DEVADDR) -DLINEBUF_MAX=$(LINEBUF_MAX)
ifneq ($(DEVICE),HANDSET)
COMMON_CFLAGS += -DBOARD_IMME_DONGLE
else
COMMON_CFLAGS += -DBOARD_IMME_HANDSET
endif

PINKOS_CFLAGS = -DPINKOS

PINKOS_DIR = pinkos

# common libraries
COMMON_LIBS = \
    common/watchdog.rel    \
    common/clock.rel       \
    common/console.rel     \
    common/radio.rel       \
    common/packet.rel

ifneq ($(DEVICE),HANDSET)
COMMON_LIBS += common/uart0.rel
else
COMMON_LIBS += common/key.rel common/spi.rel common/lcd.rel common/tiles.rel common/lcdterm.rel
endif


PINKOS_LIBS = \
    pinkos/pinkos.rel      \
    pinkos/shell.rel       \
    common/parse.rel 

COMMON_LDFLAGS = -V
PINKOS_LDFLAGS = --code-loc 0x0000

CFLAGS = $(COMMON_CFLAGS)
LDFLAGS = $(COMMON_LDFLAGS)

CFLAGS += $(PINKOS_CFLAGS)
LDFLAGS += $(PINKOS_LDFLAGS)
LIBS = $(PINKOS_LIBS)
TARGET = pinkos
DIR = pinkos

LIBS += $(COMMON_LIBS)

all: $(TARGET).hex

%.rel : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET).hex: $(LIBS)
	sdcc $(LDFLAGS) $(LIBS)
	packihx < $(DIR)/$(TARGET).ihx > $(TARGET).hex

erase:
	goodfet.cc erase

install: $(TARGET).hex
	goodfet.cc flash $(TARGET).hex
	goodfet.cc info

verify: $(TARGET).hex
	goodfet.cc verify $(TARGET).hex

clean:
	rm -f pinkos/*.hex pinkos/*.ihx pinkos/*.rel pinkos/*.asm pinkos/*.lst pinkos/*.rst pinkos/*.sym pinkos/*.lnk pinkos/*.map pinkos/*.mem
	rm -f common/*.hex common/*.ihx common/*.rel common/*.asm common/*.lst common/*.rst common/*.sym common/*.lnk common/*.map common/*.mem
	rm -f app/*.hex app/*.ihx app/*.rel app/*.asm app/*.lst app/*.rst app/*.sym app/*.lnk app/*.map app/*.mem
	rm -f *.hex *.ihx *.rel *.asm *.lst *.rst *.sym *.lnk *.map *.mem

