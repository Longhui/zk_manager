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
int lost_all_slaves(const char* uuid)
{
  fprintf(stderr, "%s lost all slaves\n", uuid);
  return 0;
}

int my_master_alive(const char* uuid)
{
  fprintf(stderr, "my repl master %s alive\n", uuid);
}

int my_master_dead(const char* uuid)
{
  fprintf(stderr, "my repl master %s dead\n", uuid);
}

// become master success
int become_master(const char* uuid)
{
  fprintf(stderr, "%s become master success\n", uuid);
  return 0;
}

// become master fail
int become_master_1(const char* uuid)
{
  fprintf(stderr, "%s become master fail\n", uuid);
  return 1;
}
// become standby
int become_standby(const char* uuid)
{
  fprintf(stderr, "%s become standby\n", uuid);
  return 0;
}


// have a replication slave
int have_a_slave(const char* uuid)
{
  fprintf(stderr, "%s have a slave\n", uuid);
  return 0;
}

set_func_void_pchar_punll set_setsyncpoint= NULL;
set_func_int_pchar_t set_replslavealive= NULL;
set_func_int_pchar_t set_replslavedead= NULL;
set_func_int_pchar_t set_replmasteralive= NULL;
set_func_int_pchar_t set_replmasterdead= NULL;
set_func_int_pchar_t set_becomemaster= NULL;
set_func_int_pchar_t set_becomestandby= NULL;

int my_set_cb_funcs()
{
  // set callback function for zk_manager
  set_setsyncpoint(get_io_syncpoint);
  set_replslavedead(lost_all_slaves);
  set_replmasterdead(my_master_dead);
  set_replslavealive(have_a_slave);
  set_replmasteralive(my_master_alive);
  set_becomemaster(become_master);
  set_becomestandby(become_standby);
  return 0;
}


int main()
{
  char *uuid="a1";
  int is_master=0;
  char binlog_name[100]={0};
  char binlog_pos[100]={0};
  void *handle;
  char *error;

  //handle = dlopen("/usr/local/lib/libzk_manager.so", RTLD_LAZY);
  handle = dlopen("./libzk_manager.so", RTLD_LAZY);
  if (!handle) {
      fprintf(stderr, "%s\n", dlerror());
      exit(EXIT_FAILURE);
  }
  dlerror(); 

  func_void_void_t zm_lock_cb= (func_void_void_t)dlsym(handle, "my_lock_cb");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  func_void_void_t zm_unlock_cb= (func_void_void_t)dlsym(handle, "my_unlock_cb");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  
  
  set_func_int_void_t set_func_set_cb_1= (set_func_int_void_t)dlsym(handle, "set_func_set_cb_1");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  zm_lock_cb();
  set_func_set_cb_1(my_set_cb_funcs); 
  zm_unlock_cb();

  // set callback function for zk_manager
  set_setsyncpoint= (set_func_void_pchar_punll)dlsym(handle, "set_cb_setsyncpoint");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  set_replslavealive= (set_func_int_pchar_t)dlsym(handle, "set_cb_replslavealive");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  set_replslavedead= (set_func_int_pchar_t)dlsym(handle, "set_cb_replslavedead");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  set_replmasteralive= (set_func_int_pchar_t)dlsym(handle, "set_cb_replmasteralive");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  set_replmasterdead= (set_func_int_pchar_t)dlsym(handle, "set_cb_replmasterdead");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  


  set_becomemaster= (set_func_int_pchar_t)dlsym(handle, "set_cb_becomemaster");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

  set_becomestandby= (set_func_int_pchar_t)dlsym(handle, "set_cb_becomestandby");
  if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }  

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
  zm_rm_repl_t zm_rm_repl;
  zm_rm_repl= (zm_rm_repl_t)dlsym(handle, "zm_rm_repl"); 
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
  
  void *zm_p_1, *zm_p_2;

  fprintf(stderr,"\n----Test register_server----\n");

  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
/*  if (NULL == zm_p_1)
  {
     exit(1);
  }
*/
  fprintf(stderr, "\n>>>server [a1] should be master<<<\n");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);

  zm_p_2= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  fprintf(stderr, "\n>>>server [a2] should be standby<<<\n");
  zm_register(zm_p_2, "a2", 3307, 0);
  sleep(1);

  fprintf(stderr,"\n----Test start/stop/rm repl----\n");

  fprintf(stderr, "\n>>>server a1 should fould have a slave<<<\n");
  zm_start_repl(zm_p_2, "db-43.photo.163.org:3306");
  sleep(1);
  
  fprintf(stderr, "\n>>>server a2 should record its syncpoint<<<\n");
  zm_stop_repl(zm_p_2, "db-43.photo.163.org:3306");
  zm_stop_repl(zm_p_2, "db-43.photo.163.org:3306");

  fprintf(stderr, "\n>>>server a1 should fould lost slave<<<\n");
  zm_rm_repl(zm_p_2, "db-43.photo.163.org:3306");
  sleep(1);

  //clean
  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);

  fprintf(stderr,"\n----Test sync master_offline----\n");
  //setup
  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);

  zm_p_2= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_2, "a2", 3307, 0);
  sleep(1);


  zm_start_repl(zm_p_2, "db-43.photo.163.org:3306");
  sleep(1);
  fprintf(stderr, "\n>>>server a2 should become master<<<\n");
  sleep(1);
  zm_disconnect(zm_p_1);
  sleep(1);

  fprintf(stderr,"\n----Test sync master_online----\n");

  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  fprintf(stderr, "\n>>>server a1 should read syncpoint successfully, and become standby<<<\n");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);
  zm_get_syncpoint(zm_p_1, binlog_name, binlog_pos);

  //clean
  zm_rm_repl(zm_p_2, "db-43.photo.163.org:3306");
  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);

  fprintf(stderr,"\n----Test sync master_offline, standby become master fail----\n");
  //setup
  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);

  zm_p_2= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_2, "a2", 3306, 0);
  sleep(1);


  zm_start_repl(zm_p_2, "db-43.photo.163.org:3306");
  sleep(1);
  set_becomemaster(become_master_1);
  fprintf(stderr, "\n>>>server a2 become master fail, it should deregister from zk_manager<<<\n");
  sleep(1);
  zm_disconnect(zm_p_1);
  sleep(1);

  //clean
  set_becomemaster(become_master);
  zm_rm_repl(zm_p_2, "db-43.photo.163.org:3306");
  zm_disconnect(zm_p_2);

  fprintf(stderr,"\n----Test async master_offline----\n");
  //setup
  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);

  zm_p_2= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_2, "a2", 3306, 0);
  sleep(1);


  zm_start_repl(zm_p_2, "db-43.photo.163.org:3306");
  zm_change_repl_mode(zm_p_1, 0);
  sleep(1);
  fprintf(stderr, "\n>>>server a2 shouldn't become master, and it should deregister from zk_manager<<<\n");
  zm_disconnect(zm_p_1);
  sleep(1);

  fprintf(stderr, "\n>>>server a1 should be master, server a2 should register again<<<\n");
  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);
  fprintf(stderr, "\n>>>server a1 should get syncpoint fail, because it do async-repl<<<\n");
  zm_get_syncpoint(zm_p_1, binlog_name, binlog_pos);
  sleep(1);
  zm_change_repl_mode(zm_p_1, 1);

  //clean
  zm_rm_repl(zm_p_2, "db-43.photo.163.org:3306");
  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);


  fprintf(stderr,"\n----Test standby_offline----\n");
  //setup
  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);

  zm_p_2= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_2, "a2", 3306, 0);
  sleep(1);

  zm_start_repl(zm_p_2, "db-43.photo.163.org:3306");
  sleep(1);
  fprintf(stderr, "\n>>>server a1 should find it lost slave<<<\n");
  zm_disconnect(zm_p_2);  
  sleep(1);

  //clean
  zm_p_2= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  zm_register(zm_p_2, "a2", 3306 ,0);
  zm_rm_repl(zm_p_2, "db-43.photo.163.org:3306");

  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);

  fprintf(stderr,"\n----Test delay register_server----\n");

  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  fprintf(stderr, "\n>>>server [a1] should be standby<<<\n");
  zm_register(zm_p_1, "a1", 3306, 1);
  sleep(1);

  zm_p_2= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  fprintf(stderr, "\n>>>server [a2] should be master<<<\n");
  zm_register(zm_p_2, "a2", 3307, 0);
  sleep(1);

  //clean
  zm_disconnect(zm_p_1);
  zm_disconnect(zm_p_2);

  fprintf(stderr,"\n----Test repeat-1 register_server----\n");

  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  fprintf(stderr, "\n>>>server [a1] should be standby<<<\n");
  zm_register(zm_p_1, "a1", 3306, 1);
  sleep(1);

  fprintf(stderr, "\n>>>server [a1] should be master<<<\n");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);

  //clean
  zm_disconnect(zm_p_1);

  fprintf(stderr,"\n----Test repeat-2 register_server----\n");

  zm_p_1= zm_connect("db-43:2181,db-52:2181,db-181:2181","2181","unittest");
  fprintf(stderr, "\n>>>server [a1] should be master<<<\n");
  zm_register(zm_p_1, "a1", 3306, 0);
  sleep(1);

  fprintf(stderr, "\n>>>server [a1] shouldn't register again<<<\n");
  zm_register(zm_p_1, "a1", 3306, 1);
  sleep(1);

  //clean
  zm_disconnect(zm_p_1);

  dlclose(handle);
}
