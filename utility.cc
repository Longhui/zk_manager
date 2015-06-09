#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "utility.h"
#include "zk_manager.h"

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


void my_print(const char* info, ...)
{
  fprintf(stderr, "[VSR HA] ");
  va_list params;
  va_start(params, info);
  vfprintf(stderr, info, params);
  va_end(params);
}

// get read_master_binlog_filename/position
func1_cb_t my_set_syncpoint=NULL;

extern "C" void set_cb_setsyncpoint(func1_cb_t func1)
{
  my_set_syncpoint= func1;
}

// loss replication slave
func2_cb_t my_repl_slave_dead= NULL;
extern "C" void set_cb_replslavedead(func2_cb_t func1)
{
  my_repl_slave_dead=func1;
}

//repl master dead
func2_cb_t my_repl_master_dead= NULL;
extern "C" void set_cb_replmasterdead(func2_cb_t func1)
{
  my_repl_master_dead= func1;
}

//repl master dead
func2_cb_t my_repl_master_alive= NULL;
extern "C" void set_cb_replmasteralive(func2_cb_t func1)
{
  my_repl_master_alive= func1;
}

// become master
func2_cb_t my_become_master= NULL;
extern "C" void set_cb_becomemaster(func2_cb_t func1)
{
  my_become_master= func1;
}

// become standby
func2_cb_t my_become_standby= NULL;
extern "C" void set_cb_becomestandby(func2_cb_t func1)
{
  my_become_standby= func1;
}


// have a replication slave
func2_cb_t my_repl_slave_alive= NULL;
extern "C" void set_cb_replslavealive(func2_cb_t func1)
{
  my_repl_slave_alive= func1;
}

extern "C" void* zm_connect(const char* host, const char* port, const char* cluster_id)
{
  zk_manager* manager= new zk_manager(host, port, cluster_id);

  int ret= manager->connect();
  if (!ret)
  {
    zk_manager_p *data= new zk_manager_p;
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
  if (ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    manager->disconnect();
    delete manager;
    delete zm;
  } else
  {
    my_print("my_zm_disconnect() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}


extern "C" int zm_register(void *data, const char* uuid, int *is_master)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->register_server(uuid, is_master);
    return ret;
  } else
  {
    my_print("my_zm_register() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}


extern "C" int zm_get_syncpoint(void *data, char* filename, char* pos)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->get_syncpoint(filename, pos);
    return ret;
  } else
  {
    my_print("my_zm_get_syncpoint() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
} 


extern "C" int zm_start_repl(void *data, const char* master_uuid)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->start_repl(master_uuid);
    return ret;
  } else
  {
    my_print("my_zm_start_repl() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}


extern "C" int zm_stop_repl(void *data, const char* master_uuid)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->stop_repl(master_uuid);
  } else
  {
    my_print("my_zm_stop_repl() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}

extern "C" int zm_rm_repl(void *data, const char* master_uuid)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->rm_repl(master_uuid);
  } else
  {
    my_print("my_zm_rm_repl() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}


extern "C" int zm_change_repl_mode(void *data, int sync)
{
  zk_manager_p* zm= (zk_manager_p *)data;
  if (ZK_MANAGER_MAGIC == zm->magic)
  {
    zk_manager *manager= (zk_manager*)(zm->ptr);
    int ret= manager->change_repl_mode(sync);
    return ret;
  } else
  {
    my_print("my_zm_change_repl_mode() fail. zk_manager ptr:%x is unvailable\n", data);
    return -1;
  }
}
