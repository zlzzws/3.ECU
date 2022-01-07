
MODBUS_LIB =/usr/local/lib/lib_feng
CROSS_COMPILE=arm-xilinx-linux-gnueabi-
TARGET       := Demo_Run
SOURCE       := ./*.c
all:$(TARGET)
$(TARGET):$(SOURCE)  libmxml.a
	$(CROSS_COMPILE)gcc $^ -o $@ -lpthread -lrt -L$(MODBUS_LIB) -lmodbus -g 
clean:
	rm -rf ./*.o $(TARGET) ./Demo*
install:
	cp ./*.o $(INSTALL_PATH)
