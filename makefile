all:test

test:main_test.o zk_manager.o utility.o vsr_functions.o
	g++ -L/usr/local/lib/ -lzookeeper_mt main_test.o vsr_functions.o zk_manager.o utility.o -o test

main_test.o:main_test.cc
	g++ -o main_test.o -c main_test.cc

zk_manager.o:zk_manager.cc
	g++ -DTHREADED -I/usr/local/include/zookeeper -o zk_manager.o -c zk_manager.cc
 
utility.o:utility.cc
	g++ -o utility.o -c utility.cc

vsr_functions.o:vsr_functions.cc
	g++ -o vsr_functions.o -c vsr_functions.cc

clean:
	rm zk_manager.o  main_test.o utility.o vsr_functions.o 
