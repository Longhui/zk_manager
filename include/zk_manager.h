#ifndef ZK_MANAGER_H
#define ZK_MANAGER_H
/*====================================
* author:	hzraolh
* date:		2015-05-04
* contact:	nanyi607rao@gmail.com
* version:	1.0
* descripation: define class zk_manager
*====================================*/
#define REPL_ASYNC 3
typedef void (*func_void_pchar_punll)(char *binlog_name, unsigned long long *binlog_pos);
typedef int (*func_int_pchar_t)(const char*);
typedef void (*func_void_void_t)();
typedef int (*func_int_void_t)();
typedef void* (*zm_connect_t)(const char* host, const char* port, const char* cluster_id);
typedef int (*zm_disconnect_t)(void *data);
typedef int (*zm_register_t)(void *data, const char* uuid, int port, int delay);
typedef int (*zm_get_syncpoint_t)(void *data, char* filename, char* pos);
typedef int (*zm_start_repl_t)(void *data, const char* master_uuid);
typedef int (*zm_stop_repl_t)(void *data, const char* master_uuid);
typedef int (*zm_rm_repl_t)(void *data, const char* master_uuid);
typedef int (*zm_change_repl_mode_t)(void *data, int sync);

typedef void (*set_func_void_pchar_punll)(func_void_pchar_punll);
typedef void (*set_func_int_pchar_t)(func_int_pchar_t);
typedef void (*set_func_int_void_t)(func_int_void_t);

extern func_void_pchar_punll my_set_syncpoint;
// loss replication slave
extern func_int_pchar_t my_repl_slave_dead;
extern func_int_pchar_t my_repl_slave_alive;

// become master
extern func_int_pchar_t my_become_master;

extern func_int_pchar_t my_repl_master_dead;
extern func_int_pchar_t my_repl_master_alive;

// become standby
extern func_int_pchar_t my_become_standby;

// have a replication slave
#endif
