# build a combined plugin that reads all formats
SUBDIRS = Text

all: subdirs

.PHONY: nomulti subdirs clean $(SUBDIRS)

nomulti:
	$(MAKE) SUBDIRS="$(SUBDIRS_NOMULTI)"

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean :
	for i in $(SUBDIRS) ; do \
	  $(MAKE) -C $$i clean; \
	done
