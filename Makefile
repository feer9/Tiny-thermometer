# Makefile for programming the ATtiny85
# modified the one generated by CrossPack

DEVICE      = attiny85
CLOCK       = 8000000
PROGRAMMER  = usbasp-clone

# for ATTiny85
# see http://www.engbedded.com/fusecalc/
# default to 1MHz, you need to use -B3 option in avrdude
FUSES_1MHZ        = -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
FUSES_1MHZ_BOD2_7 = -U lfuse:w:0x62:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m
FUSES_8MHZ        = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
FUSES_8MHZ_BOD2_7 = -U lfuse:w:0xe2:m -U hfuse:w:0xdd:m -U efuse:w:0xff:m
FUSES_16MHZ       = -U lfuse:w:0xe1:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
FUSES_DEFAULT    := $(FUSES_1MHZ)
FUSES            := $(FUSES_8MHZ)
# Remember to adjust CLOCK definition when changing fuses
# Don't use 1MHz, it gets all buggy

HEADERS    := $(wildcard include/*.h include/**/*.h)
SOURCES    := $(wildcard src/*.c)
CSOURCES   := DHT11.c timer.c onewire.c USI_TWI_Master.c ds18b20.c SSD1306_minimal.c TinyWireM.c bitmaps.c app.c main.c
CXXSOURCES := 
COBJECTS   := $(CSOURCES:%.c=obj/%.o)
CXXOBJECTS := $(CXXSOURCES:%.cpp=obj/%.o)
OBJECTS    := $(COBJECTS) $(CXXOBJECTS)

# Tune the lines below only if you know what you are doing:
AVRDUDE = avrdude -c $(PROGRAMMER) -p $(DEVICE) -B 1
CC = avr-gcc
CXX = avr-g++
CFLAGS = -Wall -Os -I./include -D__AVR_ATtiny85__ -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -std=gnu18 -W -Wstrict-prototypes -ffunction-sections -fdata-sections -ffreestanding -mcall-prologues
CXXFLAGS = -Wall -Os -I./include -D__AVR_ATtiny85__ -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -fno-threadsafe-statics
LDFLAGS = -Wl,--relax -Wl,--gc-sections
COMPILE = $(CC) $(CFLAGS) $(LDFLAGS)

# symbolic targets:
all: disasm ;
#	$(info $$CSOURCES is [${CSOURCES}]) 
#	$(info $$CXXSOURCES is [${CXXSOURCES}])
#	$(info $$OBJECTS is [${OBJECTS}])
#	$(info $$HEADERS is [${HEADERS}])

obj:
	mkdir -p obj

bin:
	mkdir -p bin

$(COBJECTS): obj/%.o: src/%.c Makefile $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(CXXOBJECTS): obj/%.o: src/%.cpp Makefile $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#.S.o:
#	$(COMPILE) -x assembler-with-cpp -c $< -o $@

#.c.s:
#	$(COMPILE) -S $< -o $@

check:
	$(AVRDUDE)

flash:	all
	$(AVRDUDE) -U flash:w:bin/main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: fuse flash

clean:
	rm -rf bin/main.hex bin/main.elf $(OBJECTS) obj bin


# file targets:
bin/main.elf: bin obj $(OBJECTS) $(HEADERS) Makefile
	$(COMPILE) -o bin/main.elf $(OBJECTS)

bin/main.hex: bin/main.elf
	rm -f bin/main.hex
	avr-objcopy -j .text -j .data -O ihex bin/main.elf bin/main.hex
	avr-size --format=avr --mcu=$(DEVICE) bin/main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	bin/main.hex
	avr-objdump -d bin/main.elf > disasm.s

cpp:
	$(COMPILE) -E main.c 

# doesn't work with avr-gcc 10.2.0
whole: $(SOURCES) $(HEADERS) Makefile
	$(CC) $(CFLAGS) --combine -fwhole-program $(LDFLAGS) -o bin/main.elf $(SOURCES)
	avr-objcopy -j .text -j .data -O ihex bin/main.elf bin/main.hex
	avr-size --format=avr --mcu=$(DEVICE) bin/main.elf

# https://p5r.uk/blog/2008/avr-gcc-optimisations.html
