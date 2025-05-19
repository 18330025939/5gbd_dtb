all: VersionInfo.h host_lib host_third host_src 
	@echo "root build done"

MAJOR_VERSION := $(shell cat version.major)
MINOR_VERSION := $(shell cat version.minor)
PATCH_VERSION := $(shell cat version.patch)

ifndef release.version
  release.version = $(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_VERSION)
endif

ifndef build.level
  build.level = $(shell date)
endif

SED_COMMAND = sed \
    -e "s/@CLIENT_VERSION@/${release.version}/g" \
	-e "s/@BUILD_TIMESTAMP@/${build.level}/g" \
	-e "s/@RT_A100_VERSION_MAJOR@/${MAJOR_VERSION}/g" \
    -e "s/@RT_A100_VERSION_MINOR@/${MINOR_VERSION}/g" \
    -e "s/@RT_A100_VERSION_PATCH@/${PATCH_VERSION}/g"

VersionInfo.h: VersionInfo.h.in
	$(SED_COMMAND) $< > $@

host_lib:
	@echo "build root/lib"
	make -C $(KT_5GBD_DTB)/lib

host_third:
	@echo "build root/third_party"
	make -C $(KT_5GBD_DTB)/third_party

host_src:
	@echo "build root/src"
	make -C $(KT_5GBD_DTB)/src

clean:
	make -C $(KT_5GBD_DTB)/src clean
	make -C $(KT_5GBD_DTB)/lib clean
	make -C $(KT_5GBD_DTB)/third_party clean