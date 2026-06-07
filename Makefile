# Makefile
CFLAGS ?= -mcpu=cortex-m4 -mthumb -Wall -O0
LDFLAGS ?= -T link.ld -nostdlib
LIBS = -lc -lm -lgcc

build: firmware.bin

# convert ELF > BIN
firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

# link object files
firmware.elf: main.o
	arm-none-eabi-gcc $(LDFLAGS) $^ $(LIBS) -o $@

# compile C source to O object
main.o: main.c
	arm-none-eabi-gcc $(CFLAGS) -c $< -o $@

flash: firmware.bin
	"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" -c port=SWD mode=UR -w $< 0x08000000 -rst

clean:
	del firmware.* *.o