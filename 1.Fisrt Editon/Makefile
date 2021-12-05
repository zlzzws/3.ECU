
MODBUS_LIB =/usr/local/lib/feng_lib/
CROSS_COMPILE=arm-xilinx-linux-gnueabi-
TARGET       := Demo_Run
SOURCE       := ./*.c
$(TARGET):$(SOURCE) libtrdp.a  libmxml.a
	$(CROSS_COMPILE)gcc $^ -o $@ -lpthread -L$(MODBUS_LIB) -lmodbus 
clean:
	rm -rf ./*.o $(TARGET) ./Demo*
install:
	cp ./*.o $(INSTALL_PATH)

#arm-xilinx-linux-gnueabi-gcc ADCDataProc.c Bram.c BramDataInter.c BramDataProc.c Crc.c DataHandle.c FileSave.c GPIOControl.c main.c ModbusSer.c ModifConfig.c Socket.c #Timer.c UpdateProgram.c xml.c -o Lmu_Demo_Run -lpthread -L/usr/local/lib/ -lmodbus
