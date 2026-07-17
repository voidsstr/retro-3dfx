TOP=..

include ../make.config

GASMOFF = gasmoff.exe

DEPS = context.h types.h attrib.h feedback.h select.h buffers.h pixel.h \
          procs.h dlist.h xform.h render.h varray.h vcache.h ctable.h \
          GL/glcore.h ns.h g_disp.h ../raster/og.h ogcache.h geom_og.h 


g_asmoff.h: $(GASMOFF) $(DEPS)
	$(GASMOFF) >g_asmoff.h

$(GASMOFF): $(DEPS) ../generate/gasmoff.c
	$(CC) $(INCLUDES) $(DEFINES) ../generate/gasmoff.c -o $(GASMOFF)


clean:
	$(RM) gasmoff.exe g_asmoff.h
