all: host_lib host_third host_src 
	@echo "root build done"

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