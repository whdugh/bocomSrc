# Add / remove libs here. Remember to config ximacfg.h accordingly
# Available libs are CxImage j2k jasper jbig jpeg png tiff zlib 

SUBDIRS = zlib jpeg tiff png CxImage j2k  jasper  jbig

subdir_all   = $(patsubst %,all-%,$(SUBDIRS))
subdir_clean = $(patsubst %,clean-%,$(SUBDIRS))

all: $(subdir_all)

clean: $(subdir_clean)

.PHONY: dummy

$(subdir_all): dummy
	$(MAKE) -C $(patsubst all-%,%,$@)

$(subdir_clean): dummy
	$(MAKE) -C $(patsubst clean-%,%,$@) clean

