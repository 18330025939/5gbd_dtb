CC=$(CROSS_COMPILE)gcc
CFLAGS += $(KT_5GBD_DTB_INCLUDE)
CFLAGS += $(KT_5GBD_DTB_BUILD)

OBJ = $(KT_5GBD_DTB)/third_party/json/cJSON.o

all: $(OBJ)

%.o: %.c	
	@if [ ! -d obj ]; then mkdir obj; fi
	$(CC) $(CFLAGS) -c $< -o obj/$(notdir $@)
	
clean: 
	-@rm -rf obj/
	-@rm -f *.ihx