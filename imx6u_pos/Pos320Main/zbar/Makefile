all: directories lib

## compile and link option
#include ../system.mk
#CC = $(CROSS_COMPILE)gcc
#LD = $(CROSS_COMPILE)ld
USER_INCLUDE = $(USER_INC) $(JSON_INC) $(ZLOG_INC)
#USER_LIB=$(USER_LIB)

OPTIMIZATION = -O2

MY_CFLAGS = -pipe
WARNINGS = -Wall -Werror -Wno-unused -Wno-parentheses -Wno-logical-not-parentheses

CFLAGS += $(USER_INCLUDE) $(OPTIMIZATION) $(WARNINGS) $(MY_CFLAGS) -I. -I.. -I./include
LDFLAGS += $(USER_LIB) -lpthread -lm

SOURCES = bch15_5.c convert.c image.c processor.c rs.c zbar_cut.c	\
	binarize.c  decoder.c  img_scanner.c  qr_finder.c  scanner.c	\
	isaac.c     qrdec.c    video.c        svg.c        null.c		\
	error.c     jpeg.c         qrdectxt.c   symbol.c		\
	config.c    refcnt.c     util.c

LIBNAME = libzbar
LIBOBJS = $(SOURCES:.c=.o)
#DYLIBNAME = $(LIBNAME).so
STLIBNAME = $(LIBNAME).a
LIBLINKER = $(CC)

lib:static
# dynamic
#dynamic: $(DYLIBNAME)
static: $(STLIBNAME)

$(LIBOBJS): CFLAGS += -fPIC -fvisibility=hidden

#DYLIBNAME_VERSIONED := $(DYLIBNAME)
#DYLIBNAME_VERSIONED := $(DYLIBNAME).$(LIBMAJOR)
#DYLIBNAME_VERSIONED := $(DYLIBNAME).$(LIBMAJOR).$(LIBMINOR)

#$(DYLIBNAME):$(LIBOBJS)
#	$(LIBLINKER) $^ -o $(DYLIBNAME) $(LDFLAGS) -shared -Wl,-soname,$(DYLIBNAME)
#	[ "$(DYLIBNAME)" = "$(DYLIBNAME_VERSIONED)" ] || cp -sf $(DYLIBNAME) $(DYLIBNAME_VERSIONED)

$(STLIBNAME):$(LIBOBJS)
	ar rcs $@ $^

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

## functions
tags:
	find . -name '*.[ch]' | xargs etags

gprof:
	$(MAKE) OPTION="-pg"

gcov:
	$(MAKE) OPTION="-fprofile-arcs -ftest-coverage"

## clean
clean:$(clean_dirs)
	-rm -f $(LIBOBJS) $(DYLIBNAME) $(STLIBNAME) *.o

all-clean:clean
	-rm -f $(DEPENDENCS) $(addsuffix .*, $(DEPENDENCS)) *~ TAGS 

.PHONY: all directories
		tags gprof gcov	upload \
		clean all-clean
