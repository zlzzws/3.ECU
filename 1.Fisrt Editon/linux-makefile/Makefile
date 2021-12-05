
TARGET       := rio
SOURCE       := ./*.c

$(TARGET):$(SOURCE) libtrdp.a  libmxml.a
	gcc -g $^ -o $@ -lpthread 
clean:
	rm -rf ./*.o
install:
	cp ./*.o $(INSTALL_PATH)

