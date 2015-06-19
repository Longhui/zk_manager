#ifndef ZK_MANAGER_H
#define ZK_MANAGER_H
/*====================================
* author:	hzraolh
* date:		2015-05-04
* contact:	nanyi607rao@gmail.com
* version:	1.0
* descripation: define class zk_manager
*====================================*/
typedef void (*func1_cb_t)(char *binlog_name, unsigned long long *binlog_pos);
typedef int (*func2_cb_t)(const char*);
typedef void* (*zm_connect_t)(const char* host, const char* port, const char* cluster_id);
typedef int (*zm_disconnect_t)(void *data);
typedef int (*zm_register_t)(void *data, const char* uuid, int port, int *is_master);
typedef int (*zm_get_syncpoint_t)(void *data, char* filename, char* pos);
typedef int (*zm_start_repl_t)(void *data, const char* master_uuid);
typedef int (*zm_stop_repl_t)(void *data, const char* master_uuid);
typedef int (*zm_rm_repl_t)(void *data, const char* master_uuid);
typedef int (*zm_change_repl_mode_t)(void *data, int sync);

typedef void (*set_func1_cb_t)(func1_cb_t);
typedef void (*set_func2_cb_t)(func2_cb_t);

extern func1_cb_t my_set_syncpoint;
// loss replication slave
extern func2_cb_t my_repl_slave_dead;
extern func2_cb_t my_repl_slave_alive;

// become master
extern func2_cb_t my_become_master;

extern func2_cb_t my_repl_master_dead;
extern func2_cb_t my_repl_master_alive;

// become standby
extern func2_cb_t my_become_standby;

// have a replication slave
#endif