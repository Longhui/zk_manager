#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "utility.h"

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

