#include <string>
#include <map>
#include <set>
#include <pthread.h>
#include "zookeeper/zookeeper.h"

#define ZK_MANAGER_MAGIC 32115891

using std::string;
using std::map;
using std::pair;
using std::set;

class zk_manager
{
  public:
    string cluster_id;
    string host;
    string port;
    string my_uuid;
    set<string> mysql_endpoints;
    zhandle_t *handler;
    map<string,string> nodes;
    set<string> active_slaves;
    string my_master_znode_id;
    bool i_am_master;
    bool compeleted_register;
    pthread_mutex_t lock;

  public:
    zk_manager(const char*, const char*, const char*);  
    ~zk_manager();
 
    int connect();
    void disconnect(); 
    int find_master_repl_znode(const char* master_endpoint, char* master_uuid);
    int get_syncpoint(char* binlog_filename, char* binlog_pos);
    int register_server(const char* uuid, int service, int delay);
    int start_repl(const char*);
    int stop_repl(const char*);
    int change_repl_mode(int sync);
    int rm_repl(const char*);
};

typedef struct zk_manager_p{
  void *ptr;
  long magic;
} zk_manager_p;

class syncPoint: public std::string {
public:
  syncPoint(const string &str):string(str){}
  syncPoint(const char* str):string(str){}
  bool operator<(const syncPoint &str) const;
};

void my_print_info(const char* info, ...);
void set_syncpoint_cb(char* binlog, unsigned long long *pos);
int become_master_cb(const char* uuid);
void become_standby_cb(const char* uuid);
void repl_master_alive_cb(const char* uuid);
void repl_master_dead_cb(const char* uuid);
void repl_slave_alive_cb(const char* uuid);
void repl_slave_dead_cb(const char* uuid);
