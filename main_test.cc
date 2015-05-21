#include "zk_manager.h"
#include "stdio.h"
#include "unistd.h"

int main()
{

  fprintf(stderr,"----test register_server----\n");

  zk_manager test("127.0.0.1","3181","test111111");
  test.connect();

  char *uuid="a1";
  int is_master=0;
  char binlog_name[100]={0};
  char binlog_pos[100]={0};

  test.register_server(uuid, &is_master, binlog_name, binlog_pos);
  fprintf(stderr," is_master:%d[1], binlog_name:%s, binlog_pos:%s\n",
           is_master, binlog_name, binlog_pos);
  sleep(1);

  zk_manager test1("127.0.0.1","3181","test111111");
  test1.connect();

  uuid="a2";
  test1.register_server(uuid, &is_master, binlog_name, binlog_pos);
  fprintf(stderr," is_master:%d[0], binlog_name:%s, binlog_pos:%s\n",
           is_master, binlog_name, binlog_pos);
 
  sleep(1);

  fprintf(stderr,"----test master_offline----\n");
  test1.start_repl("a1");
  sleep(1);
  test1.stop_repl("a1");
  sleep(1);
  test1.start_repl("a1");
  test.disconnect();
  sleep(1);

  fprintf(stderr,"----test master_online----\n");
  test.connect();
  test.register_server("a1", &is_master, binlog_name, binlog_pos);
  sleep(1);

  fprintf(stderr,"----test standby_offline----\n");
  test.start_repl("a2");
  sleep(1);
  test.disconnect();
  sleep(1);

  test1.disconnect();
  //test.disconnect();
}
