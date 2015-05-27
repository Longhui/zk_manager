#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "zk_manager.h"
#include "dlfcn.h"

// get read_master_binlog_filename/position
void get_io_syncpoint(char *binlog_name, unsigned long long *binlog_pos)
{
  *binlog_pos=1234;
  strcpy(binlog_name, "my-bin.000001");
}

// loss replication slave
void lost_all_slaves(const char* uuid)
{
  fprintf(stderr, "%s lost all slaves\n", uuid);
}

// become master from standby
void become_master(const char* uuid)
{
  fprintf(stderr, "%s become master\n", uuid);
}

// have a replication slave
void have_a_slave(const char* uuid)
{
  fprintf(stderr, "%s have a slave\n", uuid);
}


int main()
{
  char *uuid="a1";
  int is_master=0;
  char binlog_name[100]={0};
  char binlog_pos[100]={0};
  void *handle;
  char *error;

  //handle = dlopen("/styx/home/hzraolh/work/zk_manager/libzk_manager.so", RTLD_LAZY);
  handle = dlopen("./libzk_manager.so", RTLD_LAZY);
  if (!handle) {
      fprintf(stderr, "%s\n", dlerror());
      exit(EXIT_FAILURE);
  }
  dlerror(); 
  // set callback function for zk_manager
  set_func1_cb_t set_getsyncpoint;
  set_getsyncpoint= (set_func1_cb_t)dlsym(handle, "set_cb_getsyncpoint");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  
  set_getsyncpoint(get_io_syncpoint);

  set_func2_cb_t set_lostallslaves;
  set_lostallslaves= (set_func2_cb_t)dlsym(handle, "set_cb_lostallslaves");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  
  set_lostallslaves(lost_all_slaves);

  set_func2_cb_t set_becomemaster;
  set_becomemaster= (set_func2_cb_t)dlsym(handle, "set_cb_becomemaster");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  set_becomemaster(become_master);

  set_func2_cb_t set_haveaslave;
  set_haveaslave= (set_func2_cb_t)dlsym(handle, "set_cb_haveaslave");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  
  set_haveaslave(have_a_slave);


//load zk_manager functions
  zm_connect_t zm_connect;
  zm_connect= (zm_connect_t)dlsym(handle, "zm_connect");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  
  zm_disconnect_t zm_disconnect;
  zm_disconnect= (zm_disconnect_t)dlsym(handle, "zm_disconnect");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    } 

  zm_register_t zm_register;
  zm_register= (zm_register_t)dlsym(handle, "zm_register"); 
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    } 
  zm_get_syncpoint_t zm_get_syncpoint;
  zm_get_syncpoint= (zm_get_syncpoint_t)dlsym(handle, "zm_get_syncpoint"); 
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    } 
  zm_start_repl_t zm_start_repl;
  zm_start_repl= (zm_start_repl_t)dlsym(handle, "zm_start_repl"); 
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    } 
  zm_stop_repl_t zm_stop_repl;
  zm_stop_repl= (zm_stop_repl_t)dlsym(handle, "zm_stop_repl"); 
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    } 
  zm_change_repl_mode_t zm_change_repl_mode;
  zm_change_repl_mode= (zm_change_repl_mode_t)dlsym(handle, "zm_change_repl_mode"); 
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    } 


  fprintf(stderr,"\n----Test register_server----\n");

  void *zm_p_1= zm_connect("127.0.0.1","3181","test111111");
  fprintf(stderr, ">>>server [a1] should be master<<<\n");
  zm_register(zm_p_1, "a1", &is_master);
  sleep(1);

  void *zm_p_2= zm_connect("127.0.0.1","3181","test111111");
  fprintf(stderr, ">>>server [a2] should be standby<<<\n");
  zm_register(zm_p_2, "a2", &is_master);
  sleep(1);

  //clean
  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);

  fprintf(stderr,"\n----Test sync master_offline----\n");
  //setup
  zm_p_1= zm_connect("127.0.0.1","3181","test111111");
  zm_register(zm_p_1, "a1", &is_master);
  sleep(1);

  zm_p_2= zm_connect("127.0.0.1","3181","test111111");
  zm_register(zm_p_2, "a2", &is_master);
  sleep(1);


  zm_start_repl(zm_p_2, "a1");
  sleep(1);
  fprintf(stderr, ">>>server a2 should become master<<<\n");
  sleep(1);
  zm_disconnect(zm_p_1);
  sleep(1);

  fprintf(stderr,"\n----Test sync master_online----\n");

  zm_p_1= zm_connect("127.0.0.1","3181","test111111");
  fprintf(stderr, ">>>server a1 should read syncpoint successfully, and become standby<<<\n");
  zm_register(zm_p_1, "a1", &is_master);
  zm_get_syncpoint(zm_p_1, binlog_name, binlog_pos);

  //clean
  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);

  fprintf(stderr,"\n----Test async master_offline----\n");
  //setup
  zm_p_1= zm_connect("127.0.0.1","3181","test111111");
  zm_register(zm_p_1, "a1", &is_master);
  sleep(1);

  zm_p_2= zm_connect("127.0.0.1","3181","test111111");
  zm_register(zm_p_2, "a2", &is_master);
  sleep(1);


  zm_start_repl(zm_p_2, "a1");
  zm_change_repl_mode(zm_p_1, 0);
  sleep(1);
  fprintf(stderr, ">>>server a2 shouldn't become master, and it should deregister from zk_manager<<<\n");
  zm_disconnect(zm_p_1);
  sleep(1);

  fprintf(stderr, ">>>server a1 should be master, server a2 should register again<<<\n");
  zm_p_1= zm_connect("127.0.0.1","3181","test111111");
  zm_register(zm_p_1, "a1", &is_master);
  sleep(1);
  zm_change_repl_mode(zm_p_1, 1);

  //clean
  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);


  fprintf(stderr,"\n----Test standby_offline----\n");
  //setup
  zm_p_1= zm_connect("127.0.0.1","3181","test111111");
  zm_register(zm_p_1, "a1", &is_master);
  sleep(1);

  zm_p_2= zm_connect("127.0.0.1","3181","test111111");
  zm_register(zm_p_2, "a2", &is_master);
  sleep(1);

  zm_start_repl(zm_p_2, "a1");
  sleep(1);
  fprintf(stderr, ">>>server a1 should find it lost slave<<<\n");
  zm_disconnect(zm_p_2);  
  sleep(1);

  //clean
  zm_disconnect(zm_p_1);
  //test1.disconnect();

  dlclose(handle);
 }
