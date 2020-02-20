all: directories consumeser

## compile and link option
include system.mk
#CC = $(CROSS_COMPILE)gcc
#LD = $(CROSS_COMPILE)ld
USER_INCLUDE = 

OPTIMIZATION = -O2

#MY_CFLAGS = -mfloat-abi=hard -pipe
WARNINGS = -Wall -Werror -Wno-unused -Wno-error=unused-result -Werror=return-type

CFLAGS += $(USER_INCLUDE) $(OPTIMIZATION) $(WARNINGS) $(MY_CFLAGS)
LDFLAGS += $(USER_LIB) -lpthread -lm -lcrypto

SOURCES=sockfile.c afunix_udp.c common.c wrap.c nksocket.c cJSON.c base64.c ssl_3des.c sendcmd_cloud.c sendcmd.c dealcmd.c dispose.c serial.c nklcd.c nkqrscan.c gpio-intr.c gpio-utils.c  nkkeyboard.c lcd.c main.c

# dependencs
DEPDIR = .deps
MKDIR_P = mkdir -p
directories: $(DEPDIR)
$(DEPDIR):
	${MKDIR_P} $(DEPDIR)

MAKEDEPEND = $(CC) -M $(CFLAGS) -o $(patsubst %.c, $(DEPDIR)/%.p, $<) $<
%.o : %.c
	@$(MAKEDEPEND); \
		cp $(patsubst %.c, $(DEPDIR)/%.p, $<) $(patsubst %.c, $(DEPDIR)/%.d, $<); \
		sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
			-e '/^$$/ d' -e 's/$$/ :/' < $(patsubst %.c, $(DEPDIR)/%.p, $<) >> $(patsubst %.c, $(DEPDIR)/%.d, $<); \
		rm -f $(patsubst %.c, $(DEPDIR)/%.p, $<)
	$(CC) $(CFLAGS) -c $<
-include $(SOURCES:%.c=$(DEPDIR)/%.d)
DEPENDENCS = $(patsubst %.c, $(DEPDIR)/%.d, $(SOURCES))

## bin
LCD_BIN = consumeser
lcd_bin:$(LCD_BIN)
$(LCD_BIN):sockfile.o afunix_udp.o common.o  wrap.o nksocket.o cJSON.o base64.o ssl_3des.o sendcmd_cloud.o sendcmd.o dealcmd.o dispose.o  serial.o nklcd.o nkqrscan.o gpio-utils.o  nkkeyboard.o lcd.o main.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

## functions
tags:
	find . -name '*.[ch]' | xargs etags

gprof:
	$(MAKE) OPTION="-pg"

gcov:
	$(MAKE) OPTION="-fprofile-arcs -ftest-coverage"

## clean
clean:$(clean_dirs)
	-rm -f $(LCD_BIN) *.o

all-clean:clean
	-rm -f $(DEPENDENCS) $(addsuffix .*, $(DEPENDENCS)) *~ TAGS

.PHONY: all directories lcd_bin \
		tags gprof gcov	upload \
		clean all-clean subdirs $(SUBDIRS)
