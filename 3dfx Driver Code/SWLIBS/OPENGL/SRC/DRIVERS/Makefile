TOP=..

include $(TOP)/make.config

vpath %.obj $(OBJDIR)

DIRS=ddraw
ifneq ($(DRIVER),sw)
DIRS += $(DRIVER)
endif

.PHONY: $(DIRS)

all: $(DIRS)

$(DIRS):
	$(MAKEDIR) $@ "MODEL=$(MODEL)"

clean depend:
	$(MAKEDIR) ddraw "MODEL=$(MODEL)" $@
	$(MAKEDIR) s3 "MODEL=$(MODEL)" $@

