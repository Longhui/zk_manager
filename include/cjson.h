#include <string>
#include <map>
#include <set>

using std::string;
using std::map;
using std::set;

class sset : public set<string>
{
  public:
    inline const sset& operator=(const string& val)
    {
      this->erase(this->begin(), this->end());
      this->insert(val);
      return *this;
    }

    inline const sset& operator=(const set<string>& val)
    {
      this->erase(this->begin(), this->end());
      sset::iterator it;
      for (it= val.begin(); it != val.end(); it++)
      {
        this->insert(*it);
      }
      return *this;
    }
};

class cjson
{
  private:
    map<string,sset> elements;

  public:
    cjson(){};
    cjson(const string& orig);

    int load(const string& orig);
    const string dump();
    sset& operator[](const string& key);
};
