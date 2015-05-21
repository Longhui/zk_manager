/*====================================
* author:	hzraolh
* date:		2015-05-04
* contact:	nanyi607rao@gmail.com
* version:	1.0
* descripation: define class zk_manager
*====================================*/
#include <string>
#include <set>
#include "zookeeper/zookeeper.h"
using std::string;
using std::set;

class zk_manager
{
  public:
    string cluster_id;
    string host;
    string port;
    string my_uuid;
    zhandle_t *handler;
    set<string> nodes;
    set<string> slaves;
    bool i_am_master;
    string my_master_znode_id;

  public:
    zk_manager(const char*, const char*, const char*);  
 
    int connect();
    void disconnect(); 
    int register_server(const char* uuid, int *master, char* binlog_filename, char* binlog_pos);
    int start_repl(const char *master_uuid);
    int stop_repl(const char *master_uuid);
    int change_repl_mode(int sync);
    int rm_repl(const char*);
};

