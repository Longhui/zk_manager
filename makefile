ZKIDIR=/usr/local/include/zookeeper
ZKLDIR=/usr/local/lib/

CC=g++
IDIR=./include
CFLAG1= -I$(IDIR) -fPIC 
CFLAG2= -fPIC -shared -I$(ZKIDIR) -L$(ZKLDIR) -DTHREADED
LIBS= -lzookeeper_mt
ODIR=obj
SDIR=src

_OBJ = cjson.o crc32.o ifaddr.o jsmn.o utility.o zk_manager.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_DEPS = cjson.h crc32.h ifaddr.h jsmn.h utility.h zk_manager.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

$(ODIR)/%.o: $(SDIR)/%.cc $(DEPS)
	$(CC) -c -o $@ $< $(CFLAG1)

all:libzk_manager.so

libzk_manager.so: $(OBJ)
	$(CC) -o $@ $^ $(CFLAG2) $(LIBS)

test:unittest
	./unittest

unittest:unittest.cc
	$(CC) -o unittest unittest.cc -ldl -I$(IDIR) -Wwrite-strings	

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
