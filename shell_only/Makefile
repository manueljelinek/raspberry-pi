###############################################################################
#	makefile
#	 by Alex Chadwick
#
#	A makefile script for generation of raspberry pi kernel images.
###############################################################################

# The toolchain to use. arm-none-eabi works, but there does exist 
# arm-bcm2708-linux-gnueabi.
# make sure that the bin-folder is in the PATH-Variable!

ARMGNU ?= arm-linux-gnueabihf

X = 0
ifdef USB
	X = $(USB)
endif

# The intermediate directory for compiled object files.
BUILD = build/

# The directory in which source files are stored.
SOURCE = source/

# make dependant on h-files
INCLUDE = $(SOURCE)include/

# The name of the output file to generate.
TARGET = kernel.img

# The name of the assembler listing file to generate.
LIST = kernel.list

# The name of the map file to generate.
MAP = kernel.map

# The name of the linker script to use.
LINKER = kernel.ld

# The names of libraries to use.
# LIBRARIES := csud

# The names of all object files that must be generated. Deduced from the 
# assembly code files in source.
OBJECTS := $(patsubst $(SOURCE)%.s,$(BUILD)%.o,$(wildcard $(SOURCE)*.s)) $(patsubst $(SOURCE)%.c,$(BUILD)%.o,$(wildcard $(SOURCE)*.c))

# Rule to make everything.
all: $(BUILD) $(TARGET) $(LIST)

# Rule to remake everything. Does not include clean.
rebuild: clean all 

# Rule to make the listing file.
$(LIST) : $(BUILD)output.elf
	$(ARMGNU)-objdump -d $(BUILD)output.elf > $(LIST)

# Rule to make the image file.
$(TARGET) : $(BUILD)output.elf
	$(ARMGNU)-objcopy $(BUILD)output.elf -O binary $(TARGET) 

# Rule to make the elf file.
$(BUILD)output.elf : $(OBJECTS) $(LINKER)
	$(ARMGNU)-ld --no-undefined $(OBJECTS) -L. $(patsubst %,-l %,$(LIBRARIES)) -Map $(MAP) -o $(BUILD)output.elf -T $(LINKER)

# Rule to make the object files.
$(BUILD)%.o: $(SOURCE)%.s
	$(ARMGNU)-as -I $(SOURCE) $< -o $@

$(BUILD)%.o: $(SOURCE)%.c
	$(ARMGNU)-gcc -O2 -Wall -nostdlib -nostartfiles -ffreestanding -marm -mcpu=arm1176jzf-s -I $(INCLUDE) -c $< -o $@

$(BUILD):
	mkdir $@

raspbootcom :
	$(MAKE) -C ../raspbootin/raspbootcom all

load :
	sudo ../raspbootin/raspbootcom/raspbootcom /dev/ttyUSB$(X) kernel.img;
	
help :
	@echo ""
	@echo "Targets  : help        - Zeigt diesen Text an."
	@echo "           all         - Erstellt die kernel.img "
	@echo "           clean       - Löscht alle erzeugten Files "
	@echo "           rebuild     - Führt clean und all aus "
	@echo "           raspbootcom - Erstellt raspbootcom "
	@echo "           load        - Lädt das kernel.img mit raspbootcom über den USBPort 0"
	@echo "           load USB=X  - Lädt das kernel.img mit raspbootcom über den gewünschten USBPort(X)"
	@echo ""

# Rule to clean files.
clean : 
	-rm -rf $(BUILD)
	-rm -f $(TARGET)
	-rm -f $(LIST)
	-rm -f $(MAP)
