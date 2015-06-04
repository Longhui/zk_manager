all:libzk_manager.so

libzk_manager.so:zk_manager.o utility.o
	g++ -lzookeeper_mt zk_manager.o utility.o -fPIC -shared -o libzk_manager.so

zk_manager.o:zk_manager.cc
	g++ -DTHREADED -fPIC -o zk_manager.o -c zk_manager.cc
 
utility.o:utility.cc
	g++ -fPIC -o utility.o -c utility.cc

clean:
	rm libzk_manager.so zk_manager.o  utility.o 
