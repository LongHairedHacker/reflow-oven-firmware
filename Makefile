AVRMCU ?= atmega8
F_CPU ?= 16000000
ISPPORT ?= /dev/kaboard

VERSION = 0.1

# ST7735 stuff
HEADERS = avr-st7735/include/spi.h
HEADERS += avr-st7735/include/st7735.h avr-st7735/include/st7735initcmds.h
HEADERS += avr-st7735/include/st7735_gfx.h avr-st7735/include/st7735_font.h
HEADERS += avr-st7735/images/logo_bw.h avr-st7735/fonts/free_sans.h

SRC = avr-st7735/spi.c avr-st7735/st7735.c
SRC += avr-st7735/st7735_gfx.c avr-st7735/st7735_font.c

# Local stuff
SRC += main.c
TARGET = reflow-firmware
OBJDIR = bin

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

SRC_TMP = $(subst ../,,$(SRC))
OBJ = $(SRC_TMP:%.c=$(OBJDIR)/$(AVRMCU)/%.o)

CFLAGS = -I avr-st7735/include -I avr-st7735/images -I avr-st7735/fonts
CFLAGS += -Os -Wall -Wstrict-prototypes
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fshort-enums -fpack-struct -funsigned-char -funsigned-bitfields
CFLAGS += -mmcu=$(AVRMCU) -DF_CPU=$(F_CPU)UL -DVERSION=$(VERSION)

LDFLAGS = -mmcu=$(AVRMCU) -Wl,--gc-sections

all: start $(OBJDIR)/$(AVRMCU)/$(TARGET).hex size
	@echo ":: Done !"

start:
	@echo "Reflow Firmware $(VERSION)"
	@echo "=========================="
	@echo ":: Building for $(AVRMCU)"
	@echo ":: MCU operating frequency is $(F_CPU)Hz"

images/logo.h : images/logo.png utils/img_convert.py
	python3 utils/img_convert.py $< $@

images/logo_bw.h : images/logo_bw.png utils/img_convert_mono.py
	python3 utils/img_convert_mono.py $< $@

$(OBJDIR)/$(AVRMCU)/%.o : %.c $(HEADERS) Makefile
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/$(AVRMCU)/$(TARGET).elf : $(OBJ)
	$(CC) $(LDFLAGS) $+ -o $@

$(OBJDIR)/$(AVRMCU)/$(TARGET).hex : $(OBJDIR)/$(AVRMCU)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

size : $(OBJDIR)/$(AVRMCU)/$(TARGET).elf
	@echo
	@$(SIZE) --mcu=$(AVRMCU) -C $(OBJDIR)/$(AVRMCU)/$(TARGET).elf
	@echo

clean :
	@rm -rf $(OBJDIR)

flash : all
	avrdude -c arduino \
		-p $(AVRMCU) -P $(ISPPORT) \
        -U flash:w:$(OBJDIR)/$(AVRMCU)/$(TARGET).hex

test : flash
	screen $(ISPPORT) 38400
