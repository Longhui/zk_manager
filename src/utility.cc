#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "utility.h"
#include "zk_manager.h"

pthread_mutex_t lock_cb= PTHREAD_MUTEX_INITIALIZER;
                        
bool syncPoint::operator<(const syncPoint &str) const
{
    int p1= this->find(':');
    int p2= str.find(':');
    if ((-1 != p1) && (-1 != p2))
    {
      int ret= this->substr(0, p1).compare(str.substr(0, p2));
      if (0 == ret)
      {
        long long val1= atoll(this->substr(p1+1).c_str());
        long long val2= atoll(str.substr(p2+1).c_str());
        return val1 < val2 ? 1 : 0;
       } 
       else {
        return ret < 0 ? 1 : 0;
      }
    }
    int ret= this->compare(str);
    return ret < 0 ? 1 : 0;
}

static void my_print(const char* msg, const char *level)
{
 time_t t = time(0);
 char tmp[64];
 strftime( tmp, sizeof(tmp), "%y%m%d %H:%M:%S",localtime(&t) );
 fprintf(stderr, "%s [%s] [HASuite:] ", tmp, level);
}

void my_print_info(const char* msg, ...)
{
  my_print(msg, "Info");
  va_list params;
  va_start(params, msg);
  vfprintf(stderr, msg, params);
  va_end(params);
}

void my_print_warn(const char* msg, ...)
{
  my_print(msg, "Warn");
  va_list params;
  va_start(params, msg);
  vfprintf(stderr, msg, params);
  va_end(params);
}

void my_print_error(const char* msg, ...)
{
  my_print(msg, "Error");
  va_list params;
  va_start(params, msg);
  vfprintf(stderr, msg, params);
  va_end(params);
}

func_int_void_t set_cb_funcs_1= NULL;
extern "C" void set_func_set_cb_1(func_int_void_t func1)
{
  set_cb_funcs_1= func1;
}

func_int_void_t set_cb_funcs_2= NULL;
extern "C" void set_func_set_cb_2(func_int_void_t func1)
{
  set_cb_funcs_2= func1;
} 

int set_all_cb_funcs()
{//must have hold lock_cb
  if (set_cb_funcs_1)
    return set_cb_funcs_1();
  else if (set_cb_funcs_2)
    return set_cb_funcs_2();
  else
    return 1;
}
// get read_master_binlog_filename/position
func_void_pchar_punll my_set_syncpoint=NULL;
extern "C" void set_cb_setsyncpoint(func_void_pchar_punll func1)
{
  my_set_syncpoint= func1;
}

void set_syncpoint_cb(char* binlog, unsigned long long *pos)
{
  int ret= 0;
  pthread_mutex_lock(&lock_cb);
  if (!my_set_syncpoint)
  {
    ret= set_all_cb_funcs();
  }
  if (my_set_syncpoint)
  {
    pthread_mutex_unlock(&lock_cb);
    my_set_syncpoint(binlog, pos);
  } else {
    pthread_mutex_unlock(&lock_cb);
    my_print_warn("sey_syncpoint callback function isn't set\n");
  }
}


// loss replication slave
func_int_pchar_t my_repl_slave_dead= NULL;
extern "C" void set_cb_replslavedead(func_int_pchar_t func1)
{
  my_repl_slave_dead=func1;
}

void repl_slave_dead_cb(const char* uuid)
{
  int ret= 0;
  pthread_mutex_lock(&lock_cb);
  if (!my_repl_slave_dead)
  {
    ret= set_all_cb_funcs();
  }
  if (my_repl_slave_dead)
  { 
    pthread_mutex_unlock(&lock_cb);
    my_repl_slave_dead(uuid);
  }else{
    pthread_mutex_unlock(&lock_cb);
    my_print_warn("repl_slave_dead callback function isn't set\n");
  }
}

// have a replication slave
func_int_pchar_t my_repl_slave_alive= NULL;
extern "C" void set_cb_replslavealive(func_int_pchar_t func1)
{
  my_repl_slave_alive= func1;
}

void repl_slave_alive_cb(const char* uuid)
{
  int ret= 0;
  pthread_mutex_lock(&lock_cb);
  if (!my_repl_slave_alive)
  {
    ret= set_all_cb_funcs();
  }
  if (my_repl_slave_alive)
  { 
    pthread_mutex_unlock(&lock_cb);
    my_repl_slave_alive(uuid);
  } else {
    pthread_mutex_unlock(&lock_cb);
    my_print_warn("repl_slave_alive callback function isn't set\n");
  }
}

//repl master dead
func_int_pchar_t my_repl_master_dead= NULL;
extern "C" void set_cb_replmasterdead(func_int_pchar_t func1)
{
  my_repl_master_dead= func1;
}

void repl_master_dead_cb(const char* uuid)
{
  int ret= 0;
  pthread_mutex_lock(&lock_cb);
  if (!my_repl_master_dead)
  {
    ret= set_all_cb_funcs();
  }
  if (repl_master_dead_cb)
  { 
    pthread_mutex_unlock(&lock_cb);
    my_repl_master_dead(uuid);
  }else {
    pthread_mutex_unlock(&lock_cb);
    my_print_warn("repl_master_dead callback function isn't set\n");
  }
}

//repl master dead
func_int_pchar_t my_repl_master_alive= NULL;
extern "C" void set_cb_replmasteralive(func_int_pchar_t func1)
{
  my_repl_master_alive= func1;
}

void repl_master_alive_cb(const char* uuid)
{
  int ret= 0;
  pthread_mutex_lock(&lock_cb);
  if (!my_repl_master_alive)
  {
    ret= set_all_cb_funcs();
  }
  if (my_repl_master_alive)
  {
    pthread_mutex_unlock(&lock_cb);
    my_repl_master_alive(uuid);
  }
  else
  { 
    pthread_mutex_unlock(&lock_cb);
    my_print_warn("repl_master_alive callback function isn't set\n");
  }
}

// become master
func_int_pchar_t my_become_master= NULL;
extern "C" void set_cb_becomemaster(func_int_pchar_t func1)
{
  my_become_master= func1;
}

int become_master_cb(const char* uuid)
{
  int ret= 0;
  pthread_mutex_lock(&lock_cb);
  if (!my_become_master)
  {
    ret= set_all_cb_funcs();
  }
  pthread_mutex_unlock(&lock_cb);
  if (my_become_master)
  {
    pthread_mutex_unlock(&lock_cb);
    ret= my_become_master(uuid);
  }
  else
  {
    pthread_mutex_unlock(&lock_cb);
    my_print_warn("become_master callback function isn't set\n");
  }
  return ret;
}

// become standby
func_int_pchar_t my_become_standby= NULL;
extern "C" void set_cb_becomestandby(func_int_pchar_t func1)
{
  my_become_standby= func1;
}

void become_standby_cb(const char* uuid)
{
  int ret= 0;
  pthread_mutex_lock(&lock_cb);
  if (!my_become_standby)
  {
    ret= set_all_cb_funcs();
  }
  if (my_become_standby)
  {
    pthread_mutex_unlock(&lock_cb);
    my_become_standby(uuid);
  }
  else
  {
    pthread_mutex_unlock(&lock_cb);
  }
}

extern "C" void* zm_connect(const char* host, const char* port, const char* cluster_id)
{
  zk_manager* manager= new zk_manager(host, port, cluster_id);

fprintf(stderr, "11111 new zk_manager : %x \n", manager);
  int ret= manager->connect();
  if (!ret)
  {
    zk_manager_p *data= new zk_manager_p;
fprintf(stderr, "11111 new zk_manager_p : %x \n", data);
    data->ptr= (void *)manager;
    data->magic= ZK_MANAGER_MAGIC;
    return (void *)data;
  } else
  {
    return NULL;
  }
}


extern "C" int zm_disconnect(void *data)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (data && ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    manager->disconnect();
fprintf(stderr, "1111 delete zk_manager : %x \n", manager);
fprintf(stderr, "1111 delete zk_manager_p : %x \n", zm);
    delete manager;
    delete zm;
    return 0;
  } else
  {
    my_print_info("my_zm_disconnect() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}


extern "C" int zm_register(void *data, const char* uuid, int port, int delay)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (data && ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->register_server(uuid, port, delay);
    return ret;
  } else
  {
    my_print_info("my_zm_register() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}


extern "C" int zm_get_syncpoint(void *data, char* filename, char* pos)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (data && ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->get_syncpoint(filename, pos);
    return ret;
  } else
  {
    my_print_info("my_zm_get_syncpoint() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
} 


extern "C" int zm_start_repl(void *data, const char* master_uuid)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (data && ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->start_repl(master_uuid);
    return ret;
  } else
  {
    my_print_info("my_zm_start_repl() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}


extern "C" int zm_stop_repl(void *data, const char* master_uuid)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (data && ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->stop_repl(master_uuid);
    return 0;
  } else
  {
    my_print_info("my_zm_stop_repl() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}

extern "C" int zm_rm_repl(void *data, const char* master_uuid)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (data && ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->rm_repl(master_uuid);
    return 0;
  } else
  {
    my_print_info("my_zm_rm_repl() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}

extern "C" void my_lock_cb()
{
fprintf(stderr, "my_lock_cb lock_cb : %x\n", &lock_cb);
  pthread_mutex_lock(&lock_cb);
}

extern "C" void my_unlock_cb()
{
  pthread_mutex_unlock(&lock_cb);
}

extern "C" int zm_change_repl_mode(void *data, int sync)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (data && ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->change_repl_mode(sync);
    return ret;
  } else
  {
    my_print_info("my_zm_change_repl_mode() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}
