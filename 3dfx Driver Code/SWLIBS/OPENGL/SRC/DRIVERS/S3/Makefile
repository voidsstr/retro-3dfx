TOP=../..

include $(TOP)/make.config

INCLUDES += -I../ddraw
vpath %.obj $(OBJDIR)

SRCS= \
	s3vslock.c \
	s3vtexmgr.c \
	s3vcontext.c \
	s3vpick.c \
	s3vpxlfmt.c \
	s3vrgb.c \
	s3vdepth.c \
	s3vtri.c \
	s3vtxtri.c \
	s3vtestri.c \
	s3vline.c \
	s3vdevice.c

ICDSRCS = s3vmapicd.c
STANDSRCS = s3vmap.c

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
