ROOT_DIR = ../..

# size of console line buffer
LINEBUF_MAX=32

APPLICATION_OFFSET=0x4000

CC = sdcc 
COMMON_CFLAGS = -I$(ROOT_DIR)/include --no-pack-iram -DAPPLICATION_OFFSET=$(APPLICATION_OFFSET)
COMMON_CFLAGS += -DLINEBUF_MAX=$(LINEBUF_MAX)
ifneq ($(DEVICE),HANDSET)
COMMON_CFLAGS += -DBOARD_IMME_DONGLE
else
COMMON_CFLAGS += -DBOARD_IMME_HANDSET
endif

APP_CFLAGS =

# common libraries
COMMON_LIBS = \
    $(ROOT_DIR)/common/watchdog.rel    \
    $(ROOT_DIR)/common/clock.rel       \
    $(ROOT_DIR)/common/console.rel     \

ifneq ($(DEVICE),HANDSET)
COMMON_LIBS += $(ROOT_DIR)/common/uart0.rel
else
COMMON_LIBS += $(ROOT_DIR)/common/key.rel \
               $(ROOT_DIR)/common/spi.rel \
               $(ROOT_DIR)/common/lcd.rel \
               $(ROOT_DIR)/common/tiles.rel \
               $(ROOT_DIR)/common/lcdterm.rel
endif

APP_LIBS = shellapp.rel

COMMON_LDFLAGS = -V
APP_LDFLAGS = --code-loc $(APPLICATION_OFFSET)

CFLAGS = $(COMMON_CFLAGS)
LDFLAGS = $(COMMON_LDFLAGS)

CFLAGS += $(APP_CFLAGS)
LDFLAGS += $(APP_LDFLAGS)
LIBS = $(APP_LIBS)
TARGET = shellapp

LIBS += $(COMMON_LIBS)

all: $(TARGET).hex

%.rel : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET).hex: $(LIBS)
	sdcc $(LDFLAGS) $(LIBS)
	packihx < $(TARGET).ihx > $(TARGET).hex

erase:
	goodfet.cc erase

install: $(TARGET).hex
	goodfet.cc flash $(TARGET).hex
	goodfet.cc info

verify: $(TARGET).hex
	goodfet.cc verify $(TARGET).hex

clean:
	rm -f *.hex *.ihx *.rel *.asm *.lst *.rst *.sym *.lnk *.map *.mem

