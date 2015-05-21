#include <string>
using std::string;

class syncPoint: public std::string {
public:
  syncPoint(const string &str):string(str){}
  syncPoint(const char* str):string(str){}
  bool operator<(const syncPoint &str) const;
};

void my_print(const char* info, ...);
