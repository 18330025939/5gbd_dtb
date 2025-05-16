CFLAGS += $(KT_5GBD_DTB_INCLUDE)
CFLAGS += $(KT_5GBD_DTB_BUILD)

CROSS_COMPILE ?= 
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
LDFLAGS += -fPIC

LIBS += -lusb-1.0 -lcurl -lssh -levent -levent_pthreads -lpthread
LIBS += -L../lib -lpub 
LIBS += -L$(KT_5GBD_DTB)/third_party/mqtt/lib -lpaho-mqtt3a
LIBS += -L$(KT_5GBD_DTB)/third_party/spdlog -lspdlog

EXEC = 5GBD_MODULE

THIRD_PARTY = $(KT_5GBD_DTB)/third_party/json/obj/cJSON.o

SRC += $(KT_5GBD_DTB)/src/hal/serial.o
# SRC += $(KT_5GBD_DTB)/src/hal/can.o
# SRC += $(KT_5GBD_DTB)/src/hal/iic.o
# SRC += $(KT_5GBD_DTB)/src/hal/usb.o
SRC += $(KT_5GBD_DTB)/src/hal/lane_to.o
SRC += $(KT_5GBD_DTB)/src/hal/fx650.o
SRC += $(KT_5GBD_DTB)/src/hal/led.o
SRC += $(KT_5GBD_DTB)/src/units/log.o
# SRC += $(KT_5GBD_DTB)/src/net/accept_client.o
# SRC += $(KT_5GBD_DTB)/src/net/tcp_server.o
SRC += $(KT_5GBD_DTB)/src/net/mqtt_client.o
SRC += $(KT_5GBD_DTB)/src/net/ftp_handler.o
SRC += $(KT_5GBD_DTB)/src/net/tcp_client.o
SRC += $(KT_5GBD_DTB)/src/net/ssh_client.o
SRC += $(KT_5GBD_DTB)/src/services/cloud_comm.o
SRC += $(KT_5GBD_DTB)/src/services/fkz9_comm.o
SRC += $(KT_5GBD_DTB)/src/services/firmware_updater.o
SRC += $(KT_5GBD_DTB)/src/main.o


OBJ = $(addprefix obj/,$(notdir $(SRC)))

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(THIRD_PARTY) $(OBJ) $(LDFLAGS) $(LIBS)
	@if [ ! -d $(KT_5GBD_DTB)/bin ]; then mkdir -p $(KT_5GBD_DTB)/bin; fi
	-@cp -r $(EXEC) $(KT_5GBD_DTB)/bin

%.o: %.c
	@if [ ! -d obj ]; then mkdir -p obj; fi
	$(CC) $(CFLAGS) -c $< -o obj/$(notdir $@)

%.o: %.cpp
	@if [ ! -d obj ]; then mkdir -p obj; fi
	$(CXX) $(CFLAGS) -c $< -o obj/$(notdir $@)

clean:
	-@rm -f $(EXEC) *.o
	-@rm -rf obj/
	-@rm -f *.ihx
	-@rm -rf $(KT_5GBD_DTB)/bin
