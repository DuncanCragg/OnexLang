
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

LINUX_FLAGS=-g3 -ggdb
LINUX_CC_SYMBOLS = -D${TARGET} ${CHANNELS}

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
	rm -f *.linux

cleanlibs: cleanx
	rm -f libOnex*.a

############################################################################################

