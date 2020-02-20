all: directories subdirs secscreen

## compile and link option
#include system.mk
#CC = $(CROSS_COMPILE)gcc
#LD = $(CROSS_COMPILE)ld
#USER_INCLUDE = $(USER_INC) $(JSON_INC) $(ZLOG_INC)
#USER_LIB=$(USER_LIB)

OPTIMIZATION = -O2

MY_CFLAGS = -pipe -I. -Iinclude
WARNINGS = -Wall -Werror -Wno-unused

SEC_SCREEN_LIB_PATH = /home/nkty/ty16/sec_screan_lib

CFLAGS += $(USER_INCLUDE) $(OPTIMIZATION) $(WARNINGS) $(MY_CFLAGS)
LDFLAGS += $(USER_LIB) -lpthread -lm -L$(SEC_SCREEN_LIB_PATH) -ldirectfb -ldirect -lts -lfusion

SUBDIRS := 
clean_dirs := $(addprefix _clean_, $(SUBDIRS))
$(SUBDIRS):
	$(MAKE) -C $@
subdirs: $(SUBDIRS)
$(clean_dirs):
	$(MAKE) -C $(patsubst _clean_%,%,$@) clean

SOURCES = main.c afunix_udp.c cJSON.c common.c dealcmd.c dispose.c sockfile.c

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
V4L2_BIN = secscreen
v4l2_bin:$(V4L2_BIN)
$(V4L2_BIN): main.o afunix_udp.o cJSON.o common.o dealcmd.o dispose.o sockfile.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(JSON_LIB) $(ZLOG_LIB) -o $@

## functions
tags:
	find . -name '*.[ch]' | xargs etags

gprof:
	$(MAKE) OPTION="-pg"

gcov:
	$(MAKE) OPTION="-fprofile-arcs -ftest-coverage"

## clean
clean:$(clean_dirs)
	-rm -f $(V4L2_BIN) *.o

all-clean:clean
	-rm -f $(DEPENDENCS) $(addsuffix .*, $(DEPENDENCS)) *~ TAGS

.PHONY: all directories console_bin v4l2_bin \
		tags gprof gcov	upload \
		clean all-clean subdirs $(SUBDIRS)
