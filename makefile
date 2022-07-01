#-------------------------------------------------------------------------------
# nRF5 Makefile

targets:
	@grep '^[a-zA-Z0-9\.#-]\+:' makefile | grep -v '^\.' | grep -v targets | sed 's/:.*//' | uniq | sed 's/\.elf/.hex/' | sed 's/^/make clean \&\& make -j /'

#-------------------------------------------------------------------------------
# set a link to the nordic SDK, something like:
# ./sdk -> /home/<username>/nordic-platform/nRF5_SDK_16.0.0_98a08e2/

GCC_ARM_TOOLCHAIN = /home/duncan/gcc-arm/bin/
GCC_ARM_PREFIX = arm-none-eabi

PRIVATE_PEM = ../OnexKernel/doc/local/private.pem

#-------------------------------------------------------------------------------

TESTS = '1'
BUTTON ='0'
LIGHT = '0'
WATCH = '0'

EXE_SOURCES =
EXE_DEFINES =

ifeq ($(TESTS), '1')
 EXE_SOURCES += $(TESTS_SOURCES)
#EXE_DEFINES += -DLOG_TO_BLE
 EXE_DEFINES += -DLOG_TO_SERIAL
 EXE_DEFINES += -DHAS_SERIAL
#EXE_DEFINES += -DONP_CHANNEL_SERIAL
#EXE_DEFINES += -DONP_OVER_SERIAL
#EXE_DEFINES += -DONP_DEBUG
endif

ifeq ($(BUTTON), '1')
 EXE_SOURCES += $(BUTTON_SOURCES)
 EXE_DEFINES += -DLOG_TO_BLE
#EXE_DEFINES += -DLOG_TO_SERIAL
 EXE_DEFINES += -DHAS_SERIAL
 EXE_DEFINES += -DONP_CHANNEL_SERIAL
 EXE_DEFINES += -DONP_OVER_SERIAL
#EXE_DEFINES += -DONP_DEBUG
endif

ifeq ($(LIGHT), '1')
 EXE_SOURCES += $(LIGHT_SOURCES)
 EXE_DEFINES += -DLOG_TO_BLE
#EXE_DEFINES += -DLOG_TO_SERIAL
 EXE_DEFINES += -DHAS_SERIAL
 EXE_DEFINES += -DONP_CHANNEL_SERIAL
 EXE_DEFINES += -DONP_OVER_SERIAL
#EXE_DEFINES += -DONP_DEBUG
endif

ifeq ($(WATCH), '1')
 EXE_SOURCES += $(TESTS_SOURCES)
 EXE_DEFINES += -DLOG_TO_BLE
#EXE_DEFINES += -DLOG_TO_SERIAL
#EXE_DEFINES += -DHAS_SERIAL
#EXE_DEFINES += -DONP_CHANNEL_SERIAL
#EXE_DEFINES += -DONP_OVER_SERIAL
#EXE_DEFINES += -DONP_DEBUG
endif

#-------------------------------------------------------------------------------

COMMON_DEFINES = \
-DAPP_TIMER_V2 \
-DAPP_TIMER_V2_RTC1_ENABLED \
-DCONFIG_GPIO_AS_PINRESET \
-DFLOAT_ABI_HARD \
-DNRF5 \
-DNRF_SD_BLE_API_VERSION=7 \
-DSOFTDEVICE_PRESENT \
-D__HEAP_SIZE=8192 \
-D__STACK_SIZE=8192 \


COMMON_DEFINES_S132 = \
$(COMMON_DEFINES) \
-DBOARD_PINETIME \
-DNRF52832_XXAA \
-DS132 \
-DNRF52 \
-DNRF52_PAN_74 \
#-DSPI_BLOCKING \
#-DLOG_TO_GFX \



COMMON_DEFINES_S140 = \
$(COMMON_DEFINES) \
-DBOARD_PCA10059 \
-DNRF52840_XXAA \
-DS140 \



COMPILER_DEFINES_S132 = \
$(COMMON_DEFINES_S132) \
$(EXE_DEFINES) \


COMPILER_DEFINES_S140 = \
$(COMMON_DEFINES_S140) \
$(EXE_DEFINES) \


INCLUDES_S132 = \
-I./include \
-I./src/ \
-I./tests \
$(OK_INCLUDES_S132) \
$(SDK_INCLUDES_S132) \


INCLUDES_S140 = \
-I./include \
-I./src/ \
-I./tests \
$(OK_INCLUDES_S140) \
$(SDK_INCLUDES_S140) \


TESTS_SOURCES = \
./tests/test-behaviours.c \
./tests/main.c \


BUTTON_SOURCES = \
./tests/ont-examples/button-light/button.c \


LIGHT_SOURCES = \
./tests/ont-examples/button-light/light.c \


LIB_SOURCES = \
./src/behaviours.c \

#-------------------------------------------------------------------------------

OK_INCLUDES_S132 = \
-I../OnexKernel/include \
-I../OnexKernel/src/platforms/nRF5/s132 \
-I../OnexKernel/tests \


OK_INCLUDES_S140 = \
-I../OnexKernel/include \
-I../OnexKernel/src/platforms/nRF5 \
-I../OnexKernel/tests \


SDK_INCLUDES_S132 = \
-I./sdk/components/softdevice/s132/headers \
-I./sdk/components/softdevice/s132/headers/nrf52 \
-I./sdk/external/thedotfactory_fonts \
-I./sdk/components/libraries/gfx \
$(SDK_INCLUDES) \


SDK_INCLUDES_S140 = \
-I./sdk/components/softdevice/s140/headers \
-I./sdk/components/softdevice/s140/headers/nrf52 \
$(SDK_INCLUDES) \


SDK_INCLUDES = \
-I./sdk/components/boards \
-I./sdk/components/libraries/balloc \
-I./sdk/components/libraries/experimental_section_vars \
-I./sdk/components/libraries/log \
-I./sdk/components/libraries/log/src \
-I./sdk/components/libraries/memobj \
-I./sdk/components/libraries/strerror \
-I./sdk/components/libraries/util \
-I./sdk/components/toolchain/cmsis/include \
-I./sdk/integration/nrfx/ \
-I./sdk/modules/nrfx/ \
-I./sdk/modules/nrfx/hal/ \
-I./sdk/modules/nrfx/mdk \

#-------------------------------------------------------------------------------
# Targets

nrf.lib.132: libonex-lang-132.a

nrf.lib.140: libonex-lang-140.a

libonex-lang-132.a: INCLUDES=$(INCLUDES_S132)
libonex-lang-132.a: COMPILER_DEFINES=$(COMPILER_DEFINES_S132)
libonex-lang-132.a: $(LIB_SOURCES:.c=.o)
	rm -f $@
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-ar rcs $@ $^
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-strip -g $@

libonex-lang-140.a: INCLUDES=$(INCLUDES_S140)
libonex-lang-140.a: COMPILER_DEFINES=$(COMPILER_DEFINES_S140)
libonex-lang-140.a: $(LIB_SOURCES:.c=.o)
	rm -f $@
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-ar rcs $@ $^
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-strip -g $@

nrf.tests.s132: INCLUDES=$(INCLUDES_S132)
nrf.tests.s132: COMPILER_DEFINES=$(COMPILER_DEFINES_S132)
nrf.tests.s132: nrf.lib.132 $(EXE_SOURCES:.c=.o)
	rm -rf okolo
	mkdir okolo
	ar x ../OnexKernel/libonex-kernel-132.a --output okolo
	ar x             ./libonex-lang-132.a   --output okolo
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-gcc $(LINKER_FLAGS) $(LD_FILES_S132) -Wl,-Map=./onex-lang.map -o ./onex-lang.out $(EXE_SOURCES:.c=.o) okolo/*
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-size ./onex-lang.out
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O binary ./onex-lang.out ./onex-lang.bin
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O ihex   ./onex-lang.out ./onex-lang.hex

nrf.tests.s140: INCLUDES=$(INCLUDES_S140)
nrf.tests.s140: COMPILER_DEFINES=$(COMPILER_DEFINES_S140)
nrf.tests.s140: nrf.lib.140 $(EXE_SOURCES:.c=.o)
	rm -rf okolo
	mkdir okolo
	ar x ../OnexKernel/libonex-kernel-140.a --output okolo
	ar x             ./libonex-lang-140.a   --output okolo
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-gcc $(LINKER_FLAGS) $(LD_FILES_S140) -Wl,-Map=./onex-lang.map -o ./onex-lang.out $(EXE_SOURCES:.c=.o) okolo/*
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-size ./onex-lang.out
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O binary ./onex-lang.out ./onex-lang.bin
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O ihex   ./onex-lang.out ./onex-lang.hex

pinetime-erase:
	openocd -f ../OnexKernel/doc/openocd-stlink.cfg -c init -c "reset halt" -c "nrf5 mass_erase" -c "reset run" -c exit

pinetime-flash-sd:
	openocd -f ../OnexKernel/doc/openocd-stlink.cfg -c init -c "reset halt" -c "program ./sdk/components/softdevice/s132/hex/s132_nrf52_7.0.1_softdevice.hex" -c "reset run" -c exit

pinetime-flash: nrf.tests.s132
	openocd -f ../OnexKernel/doc/openocd-stlink.cfg -c init -c "reset halt" -c "program ./onex-lang.hex" -c "reset run" -c exit

pinetime-reset:
	openocd -f ../OnexKernel/doc/openocd-stlink.cfg -c init -c "reset halt" -c "reset run" -c exit

dongle-flash: nrf.tests.s140
	nrfutil pkg generate --hw-version 52 --sd-req 0xCA --application-version 1 --application ./onex-lang.hex --key-file $(PRIVATE_PEM) dfu.zip
	nrfutil dfu usb-serial -pkg dfu.zip -p /dev/ttyACM0 -b 115200

#-------------------------------------------------------------------------------

LINKER_FLAGS = -O3 -g3 -mthumb -mabi=aapcs -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs

LD_FILES_S132 = -L./sdk/modules/nrfx/mdk -T../OnexKernel/src/platforms/nRF5/s132/onex.ld
LD_FILES_S140 = -L./sdk/modules/nrfx/mdk -T../OnexKernel/src/platforms/nRF5/onex.ld

COMPILER_FLAGS = -std=c99 -O3 -g3 -mcpu=cortex-m4 -mthumb -mabi=aapcs -Wall -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin -fshort-enums

.c.o:
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-gcc $(COMPILER_FLAGS) $(COMPILER_DEFINES) $(INCLUDES) -o $@ -c $<

clean:
	find src tests -name '*.o' -o -name '*.d' | xargs rm -f
	find . -name onex.ondb | xargs rm -f
	rm -rf onex-lang.??? dfu.zip core okolo
	rm -f ,*
	@echo "------------------------------"
	@echo "files not cleaned:"
	@git ls-files --others --exclude-from=.git/info/exclude | xargs -r ls -Fla

#-------------------------------------------------------------------------------
