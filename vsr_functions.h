#ifndef VSR_FUNCTIONS_H
#define VSR_FUNCTIONS_H

// get read_master_binlog_filename/position
void get_io_syncpoint(char *binlog_name, unsigned long long *binlog_pos);

// loss replication slave
void lost_all_slaves(const char*);

// become master from standby
void become_master(const char*);

// have a replication slave
void have_a_slave(const char*);

#endif
