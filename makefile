
############################################################################################
# makefile needs consolidating - it has un-merged lines from a Nordic example at the end
############################################################################################

targets:
	@grep '^[a-zA-Z0-9\.#-]\+:' makefile | grep -v '^\.' | grep -v targets | sed 's/:.*//' | uniq | sed 's/\.elf/.hex/' | sed 's/^/make clean /'

############################################################################################

INCLUDES = \
-I./include \
-I./src/ \
-I./tests \
-I./OnexKernel/include \


C_SOURCE_FILES = \
./src/behaviours.c \


TESTS_OBJECTS = \
./tests/assert.c \
./tests/test-onexlang.c \
./tests/main.c \


############################################################################################

libOnexLang.a: COMPILE_LINE=${LINUX_FLAGS} ${CC_FLAGS} $(LINUX_CC_SYMBOLS) ${INCLUDES}
libOnexLang.a: CC=/usr/bin/gcc
libOnexLang.a: LD=/usr/bin/gcc
libOnexLang.a: AR=/usr/bin/ar
libOnexLang.a: TARGET=TARGET_LINUX
libOnexLang.a: CHANNELS=-DONP_CHANNEL_SERIAL
libOnexLang.a: $(C_SOURCE_FILES:.c=.o)
	$(AR) rcs $@ $^

tests.linux: COMPILE_LINE=${LINUX_FLAGS} ${CC_FLAGS} $(LINUX_CC_SYMBOLS) ${INCLUDES}
tests.linux: CC=/usr/bin/gcc
tests.linux: LD=/usr/bin/gcc
tests.linux: TARGET=TARGET_LINUX
tests.linux: CHANNELS=-DONP_CHANNEL_SERIAL
tests.linux: libOnexLang.a ${TESTS_OBJECTS:.c=.o}
	$(LD) ${TESTS_OBJECTS:.c=.o} -L. -lOnexLang -o $@

#############################:

linux.tests: tests.linux
	./tests.linux

linux.valgrind: tests.linux
	valgrind --leak-check=yes --undef-value-errors=no ./tests.linux

############################################################################################

BUILD_DIRECTORY = build

LINUX_FLAGS=-g3 -ggdb
LINUX_CC_SYMBOLS = -D${TARGET} ${CHANNELS}

CC_FLAGS = -c -std=gnu99 -Werror -Wall -Wextra -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer

M4_CPU = -mcpu=cortex-m4 -mthumb -mabi=aapcs
M4_CC_FLAGS = -std=c99 -MP -MD -Werror -Wall -Wextra -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fshort-enums -fno-builtin -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer -O3 -g3 -mfloat-abi=hard -mfpu=fpv4-sp-d16
NRF52_CC_SYMBOLS = -DNRF5 -DNRF52 -D${TARGET} ${CHANNELS} -DTARGET_MCU_NRF52832 -DFLOAT_ABI_HARD -DNRF52840_XXAA -D__HEAP_SIZE=8192 -D__STACK_SIZE=8192

M0_CPU = -mcpu=cortex-m0 -mthumb
M0_CC_FLAGS = -std=gnu99 -Werror -Wall -Wextra -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer -O0
NRF51_CC_SYMBOLS = -DNRF5 -DNRF51 -D${TARGET} ${CHANNELS} -DTARGET_MCU_NRF51822

CFLAGS  = -DNRF51 -DTARGET_MICRO_BIT -DBSP_DEFINES_ONLY -mcpu=cortex-m0 -mthumb -mabi=aapcs --std=gnu99 -Wall -Werror -O3 -g3 -mfloat-abi=soft -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin --short-enums

LDFLAGS = -Xlinker -Map=$(BUILD_DIRECTORY)/$(OUTPUT_FILENAME).map -mthumb -mabi=aapcs -L $(M0_TEMPLATE_PATH) -T$(LINKER_SCRIPT_16K) -mcpu=cortex-m0 -Wl,--gc-sections --specs=nano.specs -lc -lnosys

ASMFLAGS = -x assembler-with-cpp -DNRF51 -DTARGET_MICRO_BIT -DBSP_DEFINES_ONLY

M0_LD_FLAGS = $(M0_CPU) -O0 --specs=nano.specs

M4_LD_FLAGS = $(M4_CPU) -O3 -g3 -mthumb -mabi=aapcs -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs

M0_TEMPLATE_PATH := ./src/platforms/nrf5/

M4_TEMPLATE_PATH := ./src/platforms/nrf5/

LINKER_SCRIPT_16K=./src/platforms/nrf5/memory-16K-no-sd.ld

LINKER_SCRIPT_256K=./src/platforms/nrf5/memory-256K-no-sd.ld

remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

############################################################################################

GNU_INSTALL_ROOT := /home/duncan/gcc-arm
GCC_BIN=$(GNU_INSTALL_ROOT)/bin

CC      = $(GCC_BIN)/arm-none-eabi-gcc
LD      = $(GCC_BIN)/arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)/arm-none-eabi-objcopy
SIZE    = $(GCC_BIN)/arm-none-eabi-size

############################################################################################

.c.o:
	$(CC) ${COMPILE_LINE} -o $@ -c $<

############################################################################################

clean:
	-find src tests -name '*.o' -o -name '*.d' | xargs rm -f
	rm -f *.bin *.elf
	rm -f ,* core
	find . -name onex.ondb | xargs rm -f
	@echo "------------------------------"

cleanx: clean
	rm -f *.linux *.hex
	rm -rf android/build android/*/build android/*/.externalNativeBuild/ android/.gradle/*/*

cleanlibs: cleanx
	rm -f libOnex*.a

############################################################################################

