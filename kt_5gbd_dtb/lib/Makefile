
CFLAGS = -I../include  -Wall

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib

ARFLAGS = rcv

LIB_NAME = libpub.a

SRC = $(KT_5GBD_DTB)/lib/queue.o 
SRC += $(KT_5GBD_DTB)/lib/publib.o
SRC += $(KT_5GBD_DTB)/lib/list.o

OBJS = $(addprefix obj/,$(notdir $(SRC)))

.PHONY:all clean
$(LIB_NAME):$(SRC)
	$(AR) $(ARFLAGS) $(LIB_NAME) $(OBJS)
	$(RANLIB) $(LIB_NAME)
	

%.o: %.c
	@if [ ! -d obj ]; then mkdir -p obj; fi
	$(CC) $(CFLAGS) -Wp,-MMD,$@.d -c $< -o obj/$(notdir $@) 

clean:
	-@rm -f *.o *.a *.d 
	-@rm -rf obj/
	
-include .*.o.d

	
