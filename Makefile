#-------------------------------------------------------------------------------
# Ubuntu + Ubuntu Touch Makefile

targets:
	@grep '^[a-zA-Z0-9\.#-]\+:' Makefile | grep -v '^\.' | grep -v targets | sed 's/:.*//' | uniq | sed 's/\.elf/.hex/' | sed 's/^/Make clean \&\& Make /'

#-------------------------------------------------------------------------------

INCLUDES = \
-I./include \
-I./src/ \
-I./tests \
$(OK_INCLUDES) \

#-------------------------------------------------------------------------------

TESTS_SOURCES = \
./tests/test-rules.c \
./tests/main.c \


LIB_SOURCES = \
./src/edit-rules.c \


OK_INCLUDES = \
-I../OnexKernel/include \
-I../OnexKernel/tests \


#-------------------------------------------------------------------------------
# Targets

libonex-lang-arm.a: COMPILE_LINE=$(ARM_FLAGS) $(CC_FLAGS) $(ARM_CC_SYMBOLS) $(INCLUDES)
libonex-lang-arm.a: CC=/home/duncan/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-gcc
libonex-lang-arm.a: LD=/home/duncan/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-gcc
libonex-lang-arm.a: AR=/home/duncan/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-ar
libonex-lang-arm.a: TARGET=TARGET_ARM
libonex-lang-arm.a: CHANNELS=-DONP_CHANNEL_SERIAL
libonex-lang-arm.a: $(LIB_SOURCES:.c=.o)
	$(AR) rcs $@ $^

libonex-lang-x86.a: COMPILE_LINE=$(X86_FLAGS) $(CC_FLAGS) $(X86_CC_SYMBOLS) $(INCLUDES)
libonex-lang-x86.a: CC=/usr/bin/gcc
libonex-lang-x86.a: LD=/usr/bin/gcc
libonex-lang-x86.a: AR=/usr/bin/ar
libonex-lang-x86.a: TARGET=TARGET_X86
libonex-lang-x86.a: CHANNELS=-DONP_CHANNEL_SERIAL
libonex-lang-x86.a: $(LIB_SOURCES:.c=.o)
	$(AR) rcs $@ $^

tests.arm: COMPILE_LINE=$(ARM_FLAGS) $(CC_FLAGS) $(ARM_CC_SYMBOLS) $(INCLUDES)
tests.arm: CC=/home/duncan/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-gcc
tests.arm: LD=/home/duncan/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-gcc
tests.arm: TARGET=TARGET_ARM
tests.arm: CHANNELS=-DONP_CHANNEL_SERIAL
tests.arm: libonex-lang-arm.a $(TESTS_SOURCES:.c=.o)
	$(LD) $(TESTS_SOURCES:.c=.o) -pthread -L. -lonex-lang-arm -L../OnexKernel -lonex-kernel-arm -o $@

tests.x86: COMPILE_LINE=$(X86_FLAGS) $(CC_FLAGS) $(X86_CC_SYMBOLS) $(INCLUDES)
tests.x86: CC=/usr/bin/gcc
tests.x86: LD=/usr/bin/gcc
tests.x86: TARGET=TARGET_X86
tests.x86: CHANNELS=-DONP_CHANNEL_SERIAL
tests.x86: libonex-lang-x86.a $(TESTS_SOURCES:.c=.o)
	$(LD) $(TESTS_SOURCES:.c=.o) -pthread -L. -lonex-lang-x86 -L../OnexKernel -lonex-kernel-xcb -o $@

#-------------------------------------------------------------------------------

arm.lib: libonex-lang-arm.a

x86.lib: libonex-lang-x86.a

arm.tests: tests.arm
	mkdir -p ol
	cp -a ./tests.arm ol

x86.tests: tests.x86
	./tests.x86

x86.valgrind: tests.x86
	valgrind --leak-check=yes --undef-value-errors=no ./tests.x86

#-------------------------------------------------------------------------------

ARM_FLAGS=-g3 -ggdb
ARM_CC_SYMBOLS = -D$(TARGET) $(CHANNELS)

X86_FLAGS=-g3 -ggdb
X86_CC_SYMBOLS = -D$(TARGET) $(CHANNELS)

CC_FLAGS = -c -std=gnu99 -Werror -Wall -Wextra -Wno-discarded-qualifiers -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer

.c.o:
	$(CC) $(COMPILE_LINE) -o $@ -c $<

copy:
	rsync -ruav --stats --progress --delete ol/ phablet@dorold:ol

clean:
	find src tests -name '*.o' -o -name '*.d' | xargs rm -f
	rm -f ,* core
	rm -rf *.arm *.x86 ol
	find . -name onex.ondb | xargs rm -f
	@echo "------------------------------"
	@echo "files not cleaned:"
	@git ls-files --others --exclude-from=.git/info/exclude | xargs -r ls -Fla

#-------------------------------------------------------------------------------
