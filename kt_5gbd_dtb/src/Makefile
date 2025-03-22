CFLAGS += $(KT_5GBD_DTB_INCLUDE)
CFLAGS += $(KT_5GBD_DTB_BUILD)

CROSS_COMPILE ?= 
CC=$(CROSS_COMPILE)gcc
LDFLAGS += -fPIC
LIBS += -L../lib -lpub -lusb-1.0 -lcurl -levent -levent_pthreads -lpthread -L$(KT_5GBD_DTB)/third_party/mqtt/lib -lpaho-mqtt3a
EXEC = 5GBD_MODULE
#KT_HOST_LIB = $(KT_5G_ROOT)/lib/crc/crc.o
SRC = $(KT_5GBD_DTB)/src/main.o
SRC += $(KT_5GBD_DTB)/src/hal/serial.o
SRC += $(KT_5GBD_DTB)/src/hal/can.o
SRC += $(KT_5GBD_DTB)/src/hal/iic.o
SRC += $(KT_5GBD_DTB)/src/hal/usb.o
SRC += $(KT_5GBD_DTB)/src/hal/lane_to.o
SRC += $(KT_5GBD_DTB)/src/net/accept_client.o
SRC += $(KT_5GBD_DTB)/src/net/tcp_server.o
SRC += $(KT_5GBD_DTB)/src/net/mqtt_client.o
SRC += $(KT_5GBD_DTB)/src/net/ftp_handler.o
SRC += $(KT_5GBD_DTB)/src/net/tcp_client.o
SRC += $(KT_5GBD_DTB)/src/services/cloud_comm.o


OBJ = $(addprefix obj/,$(notdir $(SRC)))

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ) $(LDFLAGS) $(LIBS)
	@if [ ! -d $(KT_5GBD_DTB)/bin ]; then mkdir -p $(KT_5GBD_DTB)/bin; fi
	-@cp -r $(EXEC) $(KT_5GBD_DTB)/bin

%.o: %.c
	@if [ ! -d obj ]; then mkdir -p obj; fi
	$(CC) $(CFLAGS) -c $< -o obj/$(notdir $@)

clean:
	-@rm -f $(EXEC) *.o
	-@rm -rf obj/
	-@rm -f *.ihx
	-@rm -rf $(KT_5GBD_DTB)/bin
