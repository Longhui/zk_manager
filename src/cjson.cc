#include <stdio.h>
#include <string.h>
#include "cjson.h"
#include "jsmn.h"

cjson::cjson(const string& orig)
{
  load(orig);
}

int cjson::load(const string& orig)
{
  int r;
  jsmn_parser p;
  jsmntok_t t[128]; 
  const char *js= orig.c_str();
  jsmn_init(&p);
  r = jsmn_parse(&p, js, orig.length(), t, sizeof(t)/sizeof(t[0]));
  if (r < 0) {
  	fprintf(stderr, "cjson::load Failed to parse JSON: %d, (%s)\n", r, orig.c_str());
  	return 1;
  }

  if (r < 1 || t[0].type != JSMN_OBJECT) {
  	fprintf(stderr, "cjson::load Object expected, (%s)\n", orig.c_str());
  	return 1;
  }

  if ( r > 0)
  {
     map<string,sset>::iterator it;
     for (it= elements.begin(); it != elements.end(); ++it)
     { 
       it->second.erase(it->second.begin(), it->second.end());
     }
  }

  for (int i = 1; i < r; i++) 
  {
    if (JSMN_STRING == t[i].type)
    {
      char key[32] = {0};
      strncpy(key, js + t[i].start, t[i].end - t[i].start);

      if ( JSMN_STRING == t[i +1].type ||
            JSMN_PRIMITIVE == t[i +1].type )
      {
        char value[50] = {0};
        strncpy(value, js + t[i +1].start, t[i +1].end - t[i +1].start);
        elements[key]= value;
        i++;
      }

      if(JSMN_ARRAY == t[i+1].type)
      {
        for (int j = 0; j < t[i+1].size; j++)
        {
          jsmntok_t *g = &t[i+j+2];
          char value[50] = {0};
          strncpy(value, js + g->start, g->end - g->start);
          elements[key].insert(value);
        }
        i+= t[i+1].size + 1;
      }
    }
  }
  return 0;
}


const string cjson::dump()
{
  string target="{";
  map<string,sset>::iterator it;
  for (it= elements.begin(); it != elements.end(); ++it)
  {
    if (it->second.size() > 0)
    {
      if ((it) != elements.begin())
        target += ",";  
      target += "\"" + it->first + "\":";
      if (it->second.size() > 1)
      {
        sset::iterator it2;
        target += "[";
        for (it2= it->second.begin(); it2 != it->second.end(); ++it2)
        {
         if ((it2) != it->second.begin())
           target += ",";
          target += "\"" + *it2 + "\"";
        }
        target += "]";
      } else {
        target += "\"" + *(it->second.begin()) + "\"";
      }

    }
  }

  target += "}";
  return target;
}

sset& cjson::operator[](const string& key)
{
  return elements[key];
}

