include ./makedef

all: kt_5gbd_dtb
	@echo "build done"

kt_5gbd_dtb:
	@echo "build 5gbd_dtb"
	make -C $(KT_5GBD_DTB)

clean:
	make -C $(KT_5GBD_DTB) clean