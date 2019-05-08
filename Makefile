# Squashfs Utility Makefile
INCLUDES=-I include -I ../scl/include
INDENT_FLAGS=-br -ce -i4 -bl -bli0 -bls -c4 -cdw -ci4 -cs -nbfda -l100 -lp -prs -nlp -nut -nbfde -npsl -nss

OBJS = \
	release/main.o \
	release/info.o \
	release/util.o

all: host

internal: prepare
	@echo "  CC    src/main.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/main.c -o release/main.o
	@echo "  CC    src/info.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/info.c -o release/info.o
	@echo "  CC    src/util.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/util.c -o release/util.o
	@echo "  LD    release/squtil3"
	@$(LD) -o release/squtil3 $(OBJS) $(LDFLAGS)

prepare:
	@mkdir -p release

host:
	@make internal \
		CC=gcc \
		LD=gcc \
		CFLAGS='-c -Wall -Wextra -O3 -ffunction-sections -fdata-sections -Wstrict-prototypes' \
		LDFLAGS='-s -Wl,--gc-sections -Wl,--relax'

install:
	@cp release/squtil3 /usr/bin/squtil3

uninstall:
	@rm -f /usr/bin/squtil3

indent:
	@indent $(INDENT_FLAGS) ./*/*.h
	@indent $(INDENT_FLAGS) ./*/*.c
	@rm -rf ./*/*~

clean:
	@echo "  CLEAN ."
	@rm -rf release

analysis:
	@scan-build make
	@cppcheck --force */*.h
	@cppcheck --force */*.c

gendoc:
	@doxygen aux/doxygen.conf
