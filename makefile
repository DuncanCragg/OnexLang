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

#-------------------------------------------------------------------------------

COMPILER_DEFINES = \
-DAPP_TIMER_V2 \
-DAPP_TIMER_V2_RTC1_ENABLED \
-DCONFIG_GPIO_AS_PINRESET \
-DFLOAT_ABI_HARD \
-DNRF5 \
-DNRF_SD_BLE_API_VERSION=7 \
-DSOFTDEVICE_PRESENT \
-DBOARD_PCA10059 \
-DNRF52840_XXAA \
-DS140 \
-D__HEAP_SIZE=8192 \
-D__STACK_SIZE=8192 \
$(EXE_DEFINES) \


INCLUDES = \
-I./include \
-I./src/ \
-I./tests \
$(OK_INCLUDES_DONGLE) \
$(SDK_INCLUDES) \


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

OK_INCLUDES_DONGLE = \
-I../OnexKernel/include \
-I../OnexKernel/src/onl/nRF5/dongle \
-I../OnexKernel/tests \


SDK_INCLUDES = \
-I./sdk/components/boards \
-I./sdk/components/libraries/balloc \
-I./sdk/components/libraries/experimental_section_vars \
-I./sdk/components/libraries/log \
-I./sdk/components/libraries/log/src \
-I./sdk/components/libraries/memobj \
-I./sdk/components/libraries/strerror \
-I./sdk/components/libraries/util \
-I./sdk/components/softdevice/s140/headers \
-I./sdk/components/softdevice/s140/headers/nrf52 \
-I./sdk/components/toolchain/cmsis/include \
-I./sdk/integration/nrfx/ \
-I./sdk/modules/nrfx/ \
-I./sdk/modules/nrfx/hal/ \
-I./sdk/modules/nrfx/mdk \

#-------------------------------------------------------------------------------
# Targets

libonex-lang-nrf.a: $(LIB_SOURCES:.c=.o)
	rm -f $@
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-ar rcs $@ $^
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-strip -g $@

nrf.tests: libonex-lang-nrf.a $(EXE_SOURCES:.c=.o)
	rm -rf okolo
	mkdir okolo
	ar x ../OnexKernel/libonex-kernel-dongle.a --output okolo
	ar x             ./libonex-lang-nrf.a      --output okolo
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-gcc $(LINKER_FLAGS) $(LD_FILES_DONGLE) -Wl,-Map=./onex-lang.map -o ./onex-lang.out $(EXE_SOURCES:.c=.o) okolo/*
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-size ./onex-lang.out
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O binary ./onex-lang.out ./onex-lang.bin
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O ihex   ./onex-lang.out ./onex-lang.hex

dongle-flash: nrf.tests
	nrfutil pkg generate --hw-version 52 --sd-req 0xCA --application-version 1 --application ./onex-lang.hex --key-file $(PRIVATE_PEM) dfu.zip
	nrfutil dfu usb-serial -pkg dfu.zip -p /dev/ttyACM0 -b 115200

#-------------------------------------------------------------------------------

LINKER_FLAGS = -O3 -g3 -mthumb -mabi=aapcs -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs

LD_FILES_DONGLE = -L./sdk/modules/nrfx/mdk -T../OnexKernel/src/onl/nRF5/dongle/onex.ld

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
