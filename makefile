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
./tests/test-rules.c \
./tests/main.c \


EXE_SOURCES += $(TESTS_SOURCES)


LIB_SOURCES = \
./src/edit-rules.c \

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

nrf.tests.dongle: libonex-lang-nrf.a $(EXE_SOURCES:.c=.o)
	rm -rf okolo
	mkdir okolo
	ar x ../OnexKernel/libonex-kernel-dongle.a --output okolo
	ar x             ./libonex-lang-nrf.a      --output okolo
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-gcc $(LINKER_FLAGS) $(LD_FILES_DONGLE) -Wl,-Map=./onex-lang.map -o ./onex-lang.out $(EXE_SOURCES:.c=.o) okolo/*
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-size ./onex-lang.out
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O binary ./onex-lang.out ./onex-lang.bin
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O ihex   ./onex-lang.out ./onex-lang.hex

nrf.tests.feather-sense: libonex-lang-nrf.a $(EXE_SOURCES:.c=.o)
	rm -rf okolo
	mkdir okolo
	ar x ../OnexKernel/libonex-kernel-feather-sense.a --output okolo
	ar x             ./libonex-lang-nrf.a             --output okolo
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-gcc $(LINKER_FLAGS) $(LD_FILES_DONGLE) -Wl,-Map=./onex-lang.map -o ./onex-lang.out $(EXE_SOURCES:.c=.o) okolo/*
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-size ./onex-lang.out
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O binary ./onex-lang.out ./onex-lang.bin
	$(GCC_ARM_TOOLCHAIN)$(GCC_ARM_PREFIX)-objcopy -O ihex   ./onex-lang.out ./onex-lang.hex

feather-sense-flash: nrf.tests.feather-sense
	uf2conv.py onex-lang.hex --family 0xada52840 --output onex-lang.uf2

dongle-flash: nrf.tests.dongle
	nrfutil pkg generate --hw-version 52 --sd-req 0x00 --application-version 1 --application ./onex-lang.hex --key-file $(PRIVATE_PEM) dfu.zip
	nrfutil dfu usb-serial -pkg dfu.zip -p /dev/ttyACM0 -b 115200

#-------------------------------------------------------------------------------

LINKER_FLAGS = -O3 -g3 -mthumb -mabi=aapcs -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,--gc-sections --specs=nano.specs
LINKER_FLAGS += -Xlinker --defsym -Xlinker __BUILD_TIMEZONE_OFFSET=$$(date +'%z' | awk '{ print ($$0<0?-1:1)*((substr($$0,2,2)*3600)+(substr($$0,4,2)*60)) }')

LD_FILES_DONGLE = -L./sdk/modules/nrfx/mdk -T../OnexKernel/src/onl/nRF5/dongle/onex.ld

COMPILER_FLAGS = -std=gnu17 -O3 -g3 -mcpu=cortex-m4 -mthumb -mabi=aapcs -Wall -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin -fshort-enums

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
