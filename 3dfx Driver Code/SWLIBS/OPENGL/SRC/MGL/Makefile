TOP=..

include ../make.config

vpath %.obj $(OBJDIR)

SRCS= cosmomgl.c

OBJS=$(SRCS:.c=.obj)

all: $(OBJS)

$(OBJS): %.obj : %.c
	$(CC) $(CFLAGS) -Fo$(OBJDIR)/$*.obj -c $*.c

clean:
	$(RM) $(OBJS:%=$(OBJDIR)/%)


depend:
	$(MKDEPEND) .depend $(SRCS)

sinclude .depend
