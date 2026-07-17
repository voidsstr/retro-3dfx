TOP=../..

include $(TOP)/make.config

vpath %.obj $(OBJDIR)

SRCS= \
	ddtexmgr.c\
        ddtexel.c \
	ddtexfmt.c

ICDSRCS =
STANDSRCS =



OBJS=$(SRCS:.c=.obj)
ICDOBJS=$(ICDSRCS:.c=.obj)
STANDOBJS=$(STANDSRCS:.c=.obj)

ifeq ($(MODEL),icd)
OBJS += $(ICDOBJS)
else
OBJS += $(STANDOBJS)
endif

all: $(OBJS)

$(OBJS): %.obj : %.c
	$(CC) $(CFLAGS) -Fo$(OBJDIR)/$*.obj -c $*.c

clean:
	$(RM) $(OBJS:%=$(OBJDIR)/%) $(STANDOBJS:%=$(OBJDIR)/%) $(ICDOBJS:%=$(OBJDIR)/%)

depend:
	$(MKDEPEND) .depend $(SRCS) $(STANDSRCS) $(ICDSRCS)

sinclude .depend
