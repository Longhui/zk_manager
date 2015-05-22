#include "zk_manager.h"
#include "stdio.h"
#include "unistd.h"

int main()
{
  char *uuid="a1";
  int is_master=0;
  char binlog_name[100]={0};
  char binlog_pos[100]={0};

  zk_manager test("127.0.0.1","3181","test111111");
  zk_manager test1("127.0.0.1","3181","test111111");

  fprintf(stderr,"\n----Test register_server----\n");

  test.connect();
  fprintf(stderr, ">>>server [a1] should be master<<<\n");
  test.register_server("a1", &is_master, binlog_name, binlog_pos);
  sleep(1);

  test1.connect();
  fprintf(stderr, ">>>server [a2] should be standby<<<\n");
  test1.register_server("a2", &is_master, binlog_name, binlog_pos);
  sleep(1);

  //clean
  test.disconnect();
  test1.disconnect();

  fprintf(stderr,"\n----Test sync master_offline----\n");
  //setup
  test.connect();
  test.register_server("a1", &is_master, binlog_name, binlog_pos);
  test1.connect();
  test1.register_server("a2", &is_master, binlog_name, binlog_pos);

  test1.start_repl("a1");
  sleep(1);
  fprintf(stderr, ">>>server a2 should become master<<<\n");
  sleep(1);
  test.disconnect();
  sleep(1);

  fprintf(stderr,"\n----Test sync master_online----\n");

  test.connect();
  fprintf(stderr, ">>>server a1 should read syncpoint successfully, and become standby<<<\n");
  test.register_server("a1", &is_master, binlog_name, binlog_pos);

  //clean
  test.disconnect();
  test1.disconnect();

  fprintf(stderr,"\n----Test async master_offline----\n");
  //setup
  test.connect();
  test.register_server("a1", &is_master, binlog_name, binlog_pos);
  test1.connect();
  test1.register_server("a2", &is_master, binlog_name, binlog_pos);

  test1.start_repl("a1");
  test.change_repl_mode(0);
  sleep(1);
  fprintf(stderr, ">>>server a2 shouldn't become master, and it should deregister from zk_manager<<<\n");
  test.disconnect();
  sleep(1);

  fprintf(stderr, ">>>server a1 should be master, server a2 should register again<<<\n");
  test.connect();
  test.register_server("a1", &is_master, binlog_name, binlog_pos);
  sleep(1);
  test.change_repl_mode(1);
  //clean
  test.disconnect();
  test1.disconnect();


  fprintf(stderr,"\n----Test standby_offline----\n");
  //setup
  test.connect();
  test.register_server("a1", &is_master, binlog_name, binlog_pos);
  test1.connect();
  test1.register_server("a2", &is_master, binlog_name, binlog_pos);

  test1.start_repl("a1");
  fprintf(stderr, ">>>server a1 should find it lost slave<<<\n");
  test1.disconnect();  

  sleep(1);
  //clean
  test.disconnect();
  //test1.disconnect();
 }
