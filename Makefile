
############################################################################################

targets:
	@grep '^[a-zA-Z0-9\.#-]\+:' Makefile | grep -v '^\.' | grep -v targets | sed 's/:.*//' | uniq | sed 's/\.elf/.hex/' | sed 's/^/Make clean \&\& Make /'

############################################################################################

INCLUDES = \
-I./include \
-I./src/ \
-I./OnexKernel/tests \
-I./OnexKernel/include \


C_SOURCE_FILES = \
./src/behaviours.c \


TESTS_OBJECTS = \
./OnexKernel/tests/assert.c \
./tests/test-behaviours.c \
./tests/main.c \


BUTTON_OBJECTS = \
./tests/ont-examples/button-light/button.c \


LIGHT_OBJECTS = \
./tests/ont-examples/button-light/light.c \


############################################################################################

libonex-lang-x86.a: COMPILE_LINE=${X86_FLAGS} ${CC_FLAGS} $(X86_CC_SYMBOLS) ${INCLUDES}
libonex-lang-x86.a: CC=/usr/bin/gcc
libonex-lang-x86.a: LD=/usr/bin/gcc
libonex-lang-x86.a: AR=/usr/bin/ar
libonex-lang-x86.a: TARGET=TARGET_X86
libonex-lang-x86.a: CHANNELS=-DONP_CHANNEL_SERIAL
libonex-lang-x86.a: $(C_SOURCE_FILES:.c=.o)
	$(AR) rcs $@ $^

tests.x86: COMPILE_LINE=${X86_FLAGS} ${CC_FLAGS} $(X86_CC_SYMBOLS) ${INCLUDES}
tests.x86: CC=/usr/bin/gcc
tests.x86: LD=/usr/bin/gcc
tests.x86: TARGET=TARGET_X86
tests.x86: CHANNELS=-DONP_CHANNEL_SERIAL
tests.x86: libonex-lang-x86.a ${TESTS_OBJECTS:.c=.o}
	$(LD) ${TESTS_OBJECTS:.c=.o} -pthread -L. -lonex-lang-x86 -LOnexKernel -lonex-kernel-x86 -o $@

button.x86: COMPILE_LINE=${X86_FLAGS} ${CC_FLAGS} $(X86_CC_SYMBOLS) ${INCLUDES}
button.x86: CC=/usr/bin/gcc
button.x86: LD=/usr/bin/gcc
button.x86: TARGET=TARGET_X86
button.x86: CHANNELS=-DONP_CHANNEL_SERIAL
button.x86: libonex-lang-x86.a ${BUTTON_OBJECTS:.c=.o}
	$(LD) ${BUTTON_OBJECTS:.c=.o} -pthread -L. -lonex-lang-x86 -LOnexKernel -lonex-kernel-x86 -o $@

light.x86: COMPILE_LINE=${X86_FLAGS} ${CC_FLAGS} $(X86_CC_SYMBOLS) ${INCLUDES}
light.x86: CC=/usr/bin/gcc
light.x86: LD=/usr/bin/gcc
light.x86: TARGET=TARGET_X86
light.x86: CHANNELS=-DONP_CHANNEL_SERIAL
light.x86: libonex-lang-x86.a ${LIGHT_OBJECTS:.c=.o}
	$(LD) ${LIGHT_OBJECTS:.c=.o} -pthread -L. -lonex-lang-x86 -LOnexKernel -lonex-kernel-x86 -o $@

#############################:

x86.library: libonex-lang-x86.a

x86.tests: tests.x86
	./tests.x86

x86.valgrind: tests.x86
	valgrind --leak-check=yes --undef-value-errors=no ./tests.x86

x86.button: button.x86
	./button.x86

x86.light: light.x86
	./light.x86

############################################################################################

X86_FLAGS=-g3 -ggdb
X86_CC_SYMBOLS = -D${TARGET} ${CHANNELS}

CC_FLAGS = -c -std=gnu99 -Werror -Wall -Wextra -Wno-pointer-sign -Wno-format -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-write-strings -Wno-old-style-declaration -Wno-strict-aliasing -fno-common -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer

############################################################################################

.c.o:
	$(CC) ${COMPILE_LINE} -o $@ -c $<

#############################:

clean:
	-find src tests -name '*.o' -o -name '*.d' | xargs rm -f
	rm -f ,* core
	@echo "------------------------------"

cleanx: clean
	rm -f *.x86

cleanlibs: cleanx
	rm -f libonex*.a

############################################################################################

