#include "stdio.h"
#include "string.h"
#include "vsr_functions.h"

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
