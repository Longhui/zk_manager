/*====================================
* author:	hzraolh
* date:		2015-05-04
* contact:	nanyi607rao@gmail.com
* version:	1.0
* description:
*====================================*/

#include <list>
#include <set>
#include <string>
#include <string.h>
#include "utility.h"
#include "zk_manager.h"
#include "cjson.h"
#include "ifaddr.h"
#include "crc32.h"
#include "zookeeper/zookeeper_log.h"

using std::list;
using std::set;
using std::string;

void repl_watcher_fn(zhandle_t *zh, int type, int state, const char *path,void *ctx)
{
  if (ZOO_CHILD_EVENT == type)
  {
    struct String_vector children;
    char buffer[100];
    int buffer_len= 100;
    struct Stat stat;
    struct zk_manager *instance= (struct zk_manager*)ctx;

    string repl_znode_id= instance->cluster_id + "/replication/"+ instance->my_uuid;
    int ret= zoo_wget_children(zh, repl_znode_id.c_str(), repl_watcher_fn, (void *)ctx, &children);
    if (ZOK == ret)
    {
        set<string> my_set;
        for(int i=0; i< children.count; i++)
        {
          // get all current nodes's uuid and save in my_set
          my_set.insert(children.data[i]);
        }
        if (my_set.size() > instance->active_slaves.size())
        {//new nodes appear
          set<string>::iterator it;
          for (it= my_set.begin(); it != my_set.end(); ++it)
          {
            if (instance->active_slaves.end() == instance->active_slaves.find(*it))
            {//find new node
              int is_alive= 0;
              map<string,string>::iterator it2;
              for(it2= instance->nodes.begin(); it2 !=instance->nodes.end(); it2++)
              {// new node is available?
                if (it2->second == *it)
                  is_alive= 1;
              }

              if ( is_alive && (0 == instance->active_slaves.size()) )
              {
                repl_slave_alive_cb(instance->my_uuid.c_str()); 
              }
              instance->active_slaves.insert(*it);
            }
          }
        }
        else if (my_set.size() < instance->active_slaves.size())
        {// some node disappeared
          set<string>::iterator it;
          for (it= instance->active_slaves.begin(); it!=instance->active_slaves.end(); ++it)
          {
            if(my_set.find(*it) == my_set.end())
            {//find lost node
               int is_alive= 0;
               map<string,string>::iterator it2;
               for(it2= instance->nodes.begin(); it2 !=instance->nodes.end(); it2++)
               {// lost node is available?
                 if (it2->second == *it)
                 is_alive= 1;
               }

               instance->active_slaves.erase(*it);
               if (is_alive && (0 == instance->active_slaves.size()))
               {
                 repl_slave_dead_cb(instance->my_uuid.c_str());
               }
            }
          }
        }
    }
  }
}

/***********************************************************
* name: nodes_discrease 
* description: call when nodes' number of cluster discrease,
* params:
*  IN: zh, uuid, data 
*  OUT: 
***********************************************************/
void nodes_discrease(zhandle_t* zh, const string &uuid, void *data)
{
  char buffer[100]= {0};
  int buffer_len= 100;
  struct Stat stat;
  bool is_my_repl_master= 0;
  bool is_my_repl_slave= 0;

  my_print_info("server [%s] offline\n", uuid.c_str());

  struct zk_manager *instance= (struct zk_manager *)data;
  int i_am_master= instance->i_am_master;
  string my_master_znode_id= instance->my_master_znode_id;  

  if (uuid == instance->my_uuid)
  {
    my_print_info("Ignore zookeeper alarm of myself");
    return;
  }

  string repl_slave_id=instance->cluster_id+ "/replication/" + uuid + "/" + instance->my_uuid;

  my_print_info("check znode %s\n", repl_slave_id.c_str());
  int ret= zoo_exists(zh, repl_slave_id.c_str(), 0, &stat);
  if (ZOK == ret)
  {// the disappeared node is my master
    is_my_repl_master= 1;
    my_print_info("server [%s] is my master\n", uuid.c_str());
  } 
  repl_slave_id=instance->cluster_id+ "/replication/" + instance->my_uuid + "/" + uuid;
  my_print_info("check znode %s\n", repl_slave_id.c_str());
  ret= zoo_exists(zh, repl_slave_id.c_str(), 0, &stat);
  if (ZOK == ret)
  {// the disappeared node is my master
    is_my_repl_slave= 1;
    my_print_info("server [%s] is my slave\n", uuid.c_str());
  } 
  else if ( ZNONODE != ret )
  {
    my_print_info("nodes_discrease() check znode %s exists fail. errno:%d\n", repl_slave_id.c_str(),
              ret);
    return;
  }
  pthread_mutex_lock(&instance->lock);

  if (is_my_repl_slave)
  {
    std::set<std::string>::iterator it = instance->active_slaves.find(uuid); 
    if ( instance->active_slaves.end() != it)
      instance->active_slaves.erase(it);
    my_print_info("remove [%s] from my live slaves list\n", uuid.c_str());
    if (0 == instance->active_slaves.size())
    {
      repl_slave_dead_cb(instance->my_uuid.c_str());
    }
  }

  //the master dead, check if can I become master or not.
  if (!i_am_master && is_my_repl_master)
  { 
    repl_master_dead_cb(instance->my_uuid.c_str());

    string repl_master_id= instance->cluster_id + "/replication/" + uuid;
    my_print_info("get replication mode from znode %s\n", repl_master_id.c_str());
    ret= zoo_get(zh, repl_master_id.c_str(), 0, buffer, &buffer_len, &stat);
    if ( ZOK != ret)
    {
      my_print_info("nodes_discrease() get znode %s fail. errno:%d\n", repl_master_id.c_str(),
       ret);
      goto end;
    }

    buffer[buffer_len]=0;
    cjson object;
    object.load(buffer);
    string mode= *(object["mode"].begin());

    if ( 0 == strncmp("sync", mode.c_str(), 4) )
    {
      //i always do sync-replication with disappared node, so i can be master.
        my_print_info("server [%s] do sync-replication\n", uuid.c_str());
        if(!become_master_cb(instance->my_uuid.c_str()))
        {
          instance->i_am_master= 1;
          my_print_info("server [%s] become master successfully\n", instance->my_uuid.c_str());
        } else{
          instance->i_am_master= 0;
          my_print_info("server [%s] become master fail!!\n", instance->my_uuid.c_str());
          my_print_info("server [%s] deregister from zk_manager\n", instance->my_uuid.c_str());
          ret= zoo_delete(zh, my_master_znode_id.c_str(), -1);
          instance->my_master_znode_id= "delete_by_myself";
        } 
    } else {
      //maybe i have lost some data of master, so i shouldn't be master.
      my_print_info("server [%s] can't become master, because it do async-replication\n", instance->my_uuid.c_str());
      ret= zoo_delete(zh, my_master_znode_id.c_str(), -1);
      my_print_info("server [%s] deregister from zk_manager\n", instance->my_uuid.c_str());
      instance->my_master_znode_id= "delete_by_myself";
    }
  }

end:
  pthread_mutex_unlock(&instance->lock);
}


void nodes_increase(zhandle_t *zh, const string &uuid, void *data)
{
  struct Stat stat;
  char buffer[100] = {0};
  bool is_my_repl_slave= 0;
  bool is_my_repl_master= 0;

  my_print_info("server [%s] online\n", uuid.c_str());

  struct zk_manager *instance= (zk_manager *)data;
  string my_master_znode_id= instance->my_master_znode_id;  
  if ( uuid == instance->my_uuid)
  {
    my_print_info("Ignore zookeeper alarm of myself\n");
    return;
  }

  int i_am_master= instance->i_am_master;
  string repl_slave_id= instance->cluster_id+ "/replication/" + instance->my_uuid + "/" +uuid;
  my_print_info("check znode %s\n", repl_slave_id.c_str());
  int ret= zoo_exists(zh, repl_slave_id.c_str(), 0, &stat);
  if ( ZOK == ret )
  {// the new node is my slave
    is_my_repl_slave= 1;
    my_print_info("server [%s] is my slave\n", uuid.c_str());
  }
  repl_slave_id=instance->cluster_id+ "/replication/" + uuid + "/" + instance->my_uuid;
  my_print_info("check znode %s\n", repl_slave_id.c_str());
  ret= zoo_exists(zh, repl_slave_id.c_str(), 0, &stat);
  if ( ZOK == ret )
  {// the new node is my master
    is_my_repl_master= 1;
    my_print_info("server [%s] is my slave\n", uuid.c_str());
  }

  pthread_mutex_lock(&instance->lock);
  if ("delay_register" == my_master_znode_id)
  {
    string repl_master_id= instance->cluster_id + "/master/";
    int ret= zoo_create(zh, repl_master_id.c_str(), instance->my_uuid.c_str(), instance->my_uuid.length(),
           &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL, buffer, 100);
    if ( ZOK == ret )
    {
      instance->compeleted_register= 1;
      instance->my_master_znode_id= buffer;
      my_print_info("server [%s] become standby, znode is [%s]\n", 
                     instance->my_uuid.c_str(), buffer);
      instance->i_am_master= 0;
    }
  }

  if (is_my_repl_slave)
  {
    if (instance->active_slaves.size() == 0)
    {
      instance->active_slaves.insert(uuid);
      repl_slave_alive_cb(instance->my_uuid.c_str());
    } 
  }

  if (is_my_repl_master)
  {
    repl_master_alive_cb(instance->my_uuid.c_str());

    if ("delete_by_myself" == my_master_znode_id)
    {// create my znode blow master_znode again;
      string repl_master_id= instance->cluster_id + "/master/";
      int ret= zoo_create(zh, repl_master_id.c_str(), instance->my_uuid.c_str(), instance->my_uuid.length(),
            &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL, buffer, 100);
      if ( ZOK == ret )
      {
        my_print_info("server [%s] register at zk_manager again. znode:%s\n", instance->my_uuid.c_str(), buffer);
        my_master_znode_id= buffer;
      }
    }
  }
  pthread_mutex_unlock(&instance->lock);
}


void master_watcher_fn(zhandle_t *zh, int type, int state, const char *path,void *ctx)
{
  if (ZOO_CHILD_EVENT == type)
  {
    struct String_vector children;
    char buffer[100];
    int buffer_len= 100;
    struct Stat stat;
    struct zk_manager *instance= (struct zk_manager*)ctx;

    int ret= zoo_wget_children(zh, path, master_watcher_fn, (void *)ctx, &children);
    if ( ZOK == ret )
    {
        map<string,string> my_map;
        for(int i=0; i< children.count; i++)
        {
          // get all current nodes's uuid and save in my_set
          string znode_id=instance->cluster_id + "/master/" + children.data[i];
          buffer_len=100;
          ret= zoo_get(zh, znode_id.c_str(), 0, buffer, &buffer_len, &stat);
          if ( ZOK == ret)
          {
             buffer[buffer_len]= 0;
             my_map.insert(pair<string,string>(children.data[i], buffer));
          }
        }
        if (my_map.size() > instance->nodes.size())
        {//new nodes appear
          map<string,string>::iterator it;
          for (it= my_map.begin(); it != my_map.end(); ++it)
          {
            if (instance->nodes.end() == instance->nodes.find(it->first))
            {//can't find in nodes set
              nodes_increase(zh, it->second, (void *)ctx); 
              instance->nodes.insert(*it);
              // do something else;
            }
          } 
        }
        else if (my_map.size() < instance->nodes.size())
        {// some node disappeared
          map<string,string>::iterator it;
          for (it= instance->nodes.begin(); it!=instance->nodes.end(); ++it)
          {
            if(my_map.find(it->first) == my_map.end())
            {//can't find in nodes set
              nodes_discrease(zh, it->second, (void *)ctx); 
              instance->nodes.erase(it);
              // do something else
            }
          }
        }
    }
  }
}


zk_manager::zk_manager(const char *shost, const char* sport, const char *name)
 : i_am_master(0),
   compeleted_register(0)
{
  host=shost;
  port= sport;
  cluster_id= "/";
  cluster_id+= name;
  pthread_mutex_init(&lock, NULL);
}

zk_manager::~zk_manager()
{
  pthread_mutex_destroy(&lock);
}

void fn_watcher_g(zhandle_t* zh, int type, int state, const char* path, void* watcherCtx)
{
  if (type == ZOO_SESSION_EVENT) {
      if (state == ZOO_CONNECTED_STATE) {
          my_print_info("Connected to zookeeper service successfully!\n");
      } else if (state == ZOO_EXPIRED_SESSION_STATE) { 
          my_print_info("Zookeeper session expired!\n");
          become_standby_cb("-");
     }
  }
}


int zk_manager::connect()
{
  //string endpoint= host+":"+port;
  string endpoint= host;
  int timeout=10000;

  zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
  //handler = NULL;
  handler = zookeeper_init(endpoint.c_str(),
           fn_watcher_g, timeout, 0, (void *)"zk_manager", 0);

  if (handler == NULL) {
      my_print_info("Error when connecting to zookeeper servers...\n");
      return(EXIT_FAILURE);
    }

  struct Stat stat;
  int ret= zoo_exists(handler, cluster_id.c_str(), 0, &stat);
  
  if (ZNONODE == ret)
  {
    ret= zoo_create(handler, cluster_id.c_str(), "alive", 5, 
           &ZOO_OPEN_ACL_UNSAFE, 0, 0, 0);
    if (ret)
    {
      my_print_info("create znode %s failed !!\n", cluster_id.c_str());
      return(EXIT_FAILURE);
    }
  }

  string master_id= cluster_id + "/master";
  ret= zoo_exists(handler, master_id.c_str(), 0, &stat);
  
  if (ZNONODE == ret)
  {
    ret= zoo_create(handler, master_id.c_str(), "alive", 5, 
           &ZOO_OPEN_ACL_UNSAFE, 0, 0, 0);
    if (ret)
    {
      my_print_info("create znode %s failed !!\n", master_id.c_str());
      return(EXIT_FAILURE);
    }
  }

  string replic_id= cluster_id + "/replication";
  ret= zoo_exists(handler, replic_id.c_str(), 0, &stat);
  if (ZNONODE == ret)
  {
    ret= zoo_create(handler, replic_id.c_str(), "alive", 5, 
           &ZOO_OPEN_ACL_UNSAFE, 0, 0, 0);
    if (ret)
    {
      my_print_info("create znode %s failed !!\n", replic_id.c_str());
      return(EXIT_FAILURE);
    }
  }
  i_am_master= 0;
  return(0);
}


void zk_manager::disconnect()
{
  zookeeper_close(handler); 
  my_print_info("server [%s] disconnect with zookeeper\n", my_uuid.c_str());
  become_standby_cb(my_uuid.c_str());
}


int zk_manager::get_syncpoint(char* binlog_filename, char* binlog_pos)
{
  char buffer[100];
  struct Stat stat;
  int buffer_len= 100;

  string repl_znode_id=cluster_id + "/replication/" + my_uuid;

  int ret= zoo_get(handler, repl_znode_id.c_str(), 0, buffer, &buffer_len, &stat);

  if (ZNONODE == ret)
  {
    my_print_info("can't find znode:%s\n", repl_znode_id.c_str());
    return(EXIT_FAILURE);
  }
  else if(ZOK == ret)
  {
    buffer[buffer_len]= 0;
    cjson object;
    if(object.load(buffer))
    {
      my_print_info("get_syncpoint fail, %s is not a JSON format\n", buffer);
      return(EXIT_FAILURE);
    }
    string mode= *(object["mode"].begin());
    if (0 == strncmp("sync", mode.c_str(), 4))
    {
      struct String_vector children;
      ret= zoo_get_children(handler, repl_znode_id.c_str(), 0, &children);
      if (ZOK != ret)
      {
        my_print_info("get_syncpoint() failed, get znode %s children error, errno: %d\n",
         repl_znode_id.c_str(), ret);
        return(EXIT_FAILURE);
      }
      
      if (0 == children.count) 
      {
        my_print_info("server [%s] can't get slave sync-point, because it has no active-slave \n", my_uuid.c_str());
      }
      else {
         list<syncPoint> my_list;
         for (int i=0; i<children.count; i++)
         {
           // save all active_slaves' uuid
           active_slaves.insert(children.data[i]);
           string repl_slave_id= repl_znode_id + "/" + children.data[i];
           buffer_len=100;
           ret= zoo_get(handler, repl_slave_id.c_str(), 0, buffer, &buffer_len, &stat);
           if (ZOK == ret)
           {
             buffer[buffer_len]= 0;
             my_list.push_back(buffer);             
           }
         }
 
        //get max slave's sync-point, syncPoint like "binlog_filename:binlog_pos"
         my_list.sort();
         my_list.reverse();
         syncPoint offset= *my_list.begin();
         my_print_info("server [%s] get sync-point: %s\n", my_uuid.c_str(), offset.c_str());
         int p= offset.find(':');
         if (-1 != p)
         {
           strncpy(binlog_filename, offset.substr(0, p).c_str(), 20);
           strncpy(binlog_pos, offset.substr(p+1).c_str(), 15);
         }
      }
      
    } else {
      my_print_info("server [%s] do async-replcation when offline last time.\n", my_uuid.c_str());
      return(REPL_ASYNC);
    }
  }
  else
  {
    my_print_info("get_syncpoint failed, get znode %s error, errno: %d\n",
     repl_znode_id.c_str(), ret);
    return(EXIT_FAILURE);
  }

  return 0;
}


/***********************************************************
* name: register_server 
* description: when mysqld startup, it register on zookeeper,
* and detect itself is master or not. it can aslo get slave's
* sync-point, just when it crash last time.
* params:
*  IN: 
*    uuid: mysqld's identifier
*    service: mysqld service port
*    delay: delay register below ./master/ 
*************************************************************/

int zk_manager::register_server(const char* uuid, int service, int delay)
{
  char buffer[100];
  char buffer2[100];
  struct Stat stat;
  struct Stat stat2;
  int buffer_len= 100;
  string repl_master_id;
  struct String_vector children;
  cjson object;

  my_uuid= uuid;
  if (get_endpoints_cipher(service, mysql_endpoints))
  {
    my_print_info("get service endpoints fail!\n");
    return(EXIT_FAILURE);
  }

  pthread_mutex_lock(&lock);

  if (compeleted_register)
  {
    my_print_info("Alreadly register Successfully\n");
    pthread_mutex_unlock(&lock);
    return 0;
  }
  //register znode below /replication
  string repl_znode_id=cluster_id + "/replication/" + uuid;
  int ret= zoo_get(handler, repl_znode_id.c_str(), 0, buffer, &buffer_len, &stat);
  buffer[buffer_len]= 0;

  if (ZNONODE == ret)
  {
    object["mode"]= "sync";
    object["endpoint"]= mysql_endpoints;
    string repl_info= object.dump();

    ret= zoo_create(handler, repl_znode_id.c_str(), repl_info.c_str(),
       repl_info.length(), &ZOO_OPEN_ACL_UNSAFE, 0, 0, 0);;
    if(ZOK != ret)
    {
      my_print_info("can't create znode:%s\n", repl_znode_id.c_str());
      goto fail;
    } 
  } 
  else if (ZOK == ret)
  {
    if(object.load(buffer))
    {
      object["mode"]= "sync";
    }
    object["endpoint"]= mysql_endpoints;
    string repl_info= object.dump();
 
    ret= zoo_set(handler, repl_znode_id.c_str(), repl_info.c_str(), 
                  repl_info.length(), -1);
    if(ZOK != ret)
    {
      my_print_info("can't set znode:%s\n", repl_znode_id.c_str());
      goto fail;
    } 

    // get all active slaves
    active_slaves.clear();
    ret= zoo_wget_children(handler, repl_znode_id.c_str(), repl_watcher_fn, (void *)this, &children);
    if (ZOK == ret) 
    {
      for (int i=0; i< children.count; i++)
      {
        for(map<string,string>::iterator it= nodes.begin(); it!=nodes.end(); ++it)
        {
          if (it->second == children.data[i])
            active_slaves.insert(children.data[i]);
        }
      }
    }   
  }

  //register znode below /master
  repl_master_id= cluster_id + "/master";
  ret= zoo_get_children(handler, repl_master_id.c_str(), 0, &children);
  delay &= children.count > 0 ? 0 : 1; //if there is other servers running, no need to delay register

  ret= ZOK;
  repl_master_id= cluster_id + "/master/";
  if (!delay)
  {
    ret= zoo_create(handler, repl_master_id.c_str(), uuid, strlen(uuid),
           &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL, buffer, 100);
    if ( ZOK == ret )
    { 
      compeleted_register= 1;
      my_print_info("server [%s] register at zk-mananger. znode:%s\n", my_uuid.c_str(), buffer);
      my_master_znode_id= buffer;
    } else {
      my_print_info("register_server failed, create znode %s fail, errno:%d\n", repl_master_id.c_str(), ret);
      goto fail;
    }
  }else{
    my_print_info("server [%s] delay to register znode below master/\n", my_uuid.c_str());
    my_master_znode_id= "delay_register";
  }

  // get all active znodes below /master and watch it.
  nodes.clear();
  repl_master_id= cluster_id + "/master";
  ret= zoo_wget_children(handler, repl_master_id.c_str(), master_watcher_fn, (void *)this, &children);
  if ( ZOK == ret )
  {
    if (children.count > 0)
    {
      list<string> my_list;
      for(int i=0; i< children.count; i++)
      {
        my_list.push_back(children.data[i]);
        string znode_id= repl_master_id + "/" + children.data[i];
        buffer_len=100;
        ret= zoo_get(handler, znode_id.c_str(), 0, buffer2, &buffer_len, &stat2);
        // get all current nodes's znode and uuid, save in nodes list
        if ( ZOK == ret)
        {
          buffer2[buffer_len]=0;
          nodes.insert(pair<string, string>(children.data[i], buffer2));
        }
      }

      i_am_master= 0;
      if (!delay)
      { 
        my_list.sort();
        buffer_len=100;
        string master_1st_node_id= cluster_id + "/master/" + *my_list.begin();
        ret= zoo_get(handler, master_1st_node_id.c_str(), 0, buffer, &buffer_len, &stat);
        buffer[buffer_len]= 0;
        if (ZOK == ret)
        {
          if ( my_uuid == buffer )
          {
            i_am_master= 1;
            become_master_cb(my_uuid.c_str());
            my_print_info("server [%s] become master\n", my_uuid.c_str());
          }
          else 
          {
            i_am_master= 0;
            become_standby_cb(my_uuid.c_str());

            my_print_info("server [%s] become standby, current master is server [%s]\n", my_uuid.c_str(), buffer);
          }
        }
        else
        {
          my_print_info("register_server() zoo_get fail. errno:%d\n", ret);
          goto fail;
        }
      }
    }
  }

end:
  pthread_mutex_unlock(&lock);
  return 0;

fail:  
  pthread_mutex_unlock(&lock);
  return(EXIT_FAILURE);
}

/*
  master_endpoint's format likes IP:PORT
*/
int zk_manager::find_master_repl_znode(const char* master_endpoint, char* master_uuid)
{
  struct String_vector children;
  char buffer[100];
  int buffer_len= 100;
  struct Stat stat;
  cjson object;

  char endpoint_cp[20]= {0};
  int cipher= crc32(0, master_endpoint, strlen(master_endpoint));
  sprintf(endpoint_cp, "%x", cipher);
 
  string repl_znode= cluster_id + "/replication";

  int ret= zoo_get_children(handler, repl_znode.c_str(), 0, &children);
  if ( ZOK == ret )
  {
    for(int i=0; i< children.count; i++)
    {
      //get all masters' replication information 
      string znode_id=cluster_id + "/replication/" + children.data[i];
      ret= zoo_get(handler, znode_id.c_str(), 0, buffer, &buffer_len, &stat);
      if ( ZOK == ret)
      {
         buffer[buffer_len]= 0;
         if (object.load(buffer))
         {
           my_print_info("znode %s content %s isn't JSON format\n", znode_id.c_str(), buffer);
           return(EXIT_FAILURE);
         }
         if (object["endpoint"].find(endpoint_cp) != object["endpoint"].end())
         {
           strncpy(master_uuid, children.data[i], strlen(children.data[i]));
           return 0;
         }
      }
    }
  }
  my_print_info("Can't find replication znode: %s (%s)\n", master_endpoint, endpoint_cp);
  return(EXIT_FAILURE);
}


int zk_manager::start_repl(const char* master_endpoint)
{
  struct Stat stat;
  char master_uuid[30] = {0};
  if (find_master_repl_znode(master_endpoint, master_uuid))
  {
    my_print_info("find_master_repl_znode failed!, master endpoint:%s\n", master_endpoint);
    return(EXIT_FAILURE);
  }
  string repl_slave_id= cluster_id + "/replication/" + master_uuid + "/" + my_uuid;
  char binlog_name[100]= {0};
  unsigned long long binlog_pos;
  set_syncpoint_cb(binlog_name, &binlog_pos);

  char syncpoint[100]= {0};
  sprintf(syncpoint, "%s:%lld", binlog_name, binlog_pos);
  int ret= zoo_exists(handler, repl_slave_id.c_str(), 0, &stat);
  if (ZNONODE == ret)
  {
    ret= zoo_create(handler, repl_slave_id.c_str(), syncpoint, strlen(syncpoint),
                  &ZOO_OPEN_ACL_UNSAFE, 0, 0, 0);
    if (ZOK != ret)
    {
      my_print_info("create znode: %s fail. errno:%d\n", repl_slave_id.c_str(), ret);
      return(EXIT_FAILURE);
    }
    my_print_info("record %s->%s replication on zookeeper\n", master_uuid, my_uuid.c_str());
  }
  return(0);
}


int zk_manager::stop_repl(const char* master_endpoint)
{
  // record my sync-point in slave znode
  char binlog_name[100]={0};
  unsigned long long binlog_pos= 0;
  set_syncpoint_cb(binlog_name, &binlog_pos);

  char master_uuid[30] = {0};
  if (find_master_repl_znode(master_endpoint, master_uuid))
  {
    my_print_info("find_master_repl_znode failed!, master endpoint:%s\n", master_endpoint);
    return(EXIT_FAILURE);
  }

  char syncpoint[100]={0};
  sprintf(syncpoint, "%s:%lld", binlog_name, binlog_pos);
  string repl_slave_id=cluster_id+ "/replication/" + master_uuid + "/" + my_uuid;
  int ret= zoo_set(handler, repl_slave_id.c_str(), syncpoint, 100, -1);
  if ( ZOK != ret)
  {
    my_print_info("record syncpoint: %s to znode: %s fail. errno: %d\n", syncpoint,
              repl_slave_id.c_str(), ret);
    return(EXIT_FAILURE);
  }
  else
  {
    my_print_info("record %s[m]->%s[s] syncpoint: %s on zookeeper \n", master_uuid, my_uuid.c_str(),
      syncpoint);
  }
  return(0);
}


int zk_manager::rm_repl(const char* master_endpoint)
{
  char master_uuid[30] = {0};
  if (find_master_repl_znode(master_endpoint, master_uuid))
  {
    my_print_info("find_master_repl_znode failed!, endpoint:%s\n", master_endpoint);
    return(EXIT_FAILURE);
  }
  string repl_slave_id= cluster_id + "/replication/" + master_uuid + "/" + my_uuid;
  int ret= zoo_delete(handler, repl_slave_id.c_str(), -1);
  if (ZOK != ret)
  {
    my_print_info("delete znode: %s fail. errno:%d\n", repl_slave_id.c_str(), ret);
    return(EXIT_FAILURE);
  }
  my_print_info("remove %s[m]->%s[s] replication on zookeeper\n", master_uuid, my_uuid.c_str());
  return(0);
}


int zk_manager::change_repl_mode(int sync)
{
  string repl_master_id= cluster_id + "/replication/" + my_uuid;
  cjson object;
  object["endpoint"]= mysql_endpoints;
  object["mode"]= sync > 0 ? "sync" : "async";
  string repl_info= object.dump();

  int ret= zoo_set(handler, repl_master_id.c_str(), repl_info.c_str(), repl_info.length(), -1);
  if (ZOK != ret)
  { 
    my_print_info("change znode: %s value to %s fail. errno: %d\n", repl_master_id.c_str(), sync > 0 ? "sync": "async", ret);
    return(EXIT_FAILURE);
  }
  else 
  {
    my_print_info("change server [%s] repl mode to %s\n", my_uuid.c_str(), sync > 0 ? "sync" : "async");
  }
  return(0);
}

