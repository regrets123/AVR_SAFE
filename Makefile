CC      := avr-gcc
LD      := avr-ld
OBJCOPY := avr-objcopy
OBJISP  := avrdude
AVRSIZE := avr-size
PORT    := \\\\.\\COM6
# För att kunna flasha måste detta kopplas till den port arduinon är inkopplad till
# På Linux kan man hitta det genom att köra kommandot <sudo dmesg | grep tty>
# På Windows får man nog kolla i device managern/enhetshanteraren, och det ser ut så här för COM3 som ett exempel: PORT := \\\\.\\COM3

MCU := atmega328p

SRC_DIR  := src
BIN_DIR  := bin
OBJ_DIR  := obj

CFLAGS  := -Wall -Wextra  -Wundef -pedantic \
		-Os -std=gnu99 -DF_CPU=16000000UL -mmcu=${MCU} -DBAUD=9600 -fshort-enums
LDFLAGS := -mmcu=$(MCU)

BIN := program.hex
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: clean fresh all isp

all: $(BIN_DIR)/$(BIN)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(COMPILE.c) -MD -o $@ -c $<

%.elf: $(OBJ) | $(BIN_DIR)
	$(CC) -Wl,-Map=$(@:.elf=.map) $(LDFLAGS) -o $@ $^
	$(AVRSIZE) $@
	@mv $(BIN_DIR)/*.map $(OBJ_DIR)

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

isp: $(BIN_DIR)/${BIN}
	$(OBJISP) -F -V -c arduino -p ${MCU} -P ${PORT} -U flash:w:$<

clean:
	@rm -rv $(OBJ_DIR)

fresh: clean
	@rm -rv $(BIN_DIR)

$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@
