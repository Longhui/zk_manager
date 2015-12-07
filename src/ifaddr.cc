#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ifaddr.h"
#include "crc32.h"

static int get_ifaddrs(set<string>& addr_list)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return -1;
    }
    /* Walk through linked list, maintaining head pointer so we
     * can free list later */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;
       //if (family == AF_INET || family == AF_INET6) {
        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                    sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                fprintf(stderr, "getnameinfo() failed: %s\n", gai_strerror(s));
                return -1;
            }
            if (0 != strcmp(host, "127.0.0.1"))
              addr_list.insert(host);
        }
    }
    freeifaddrs(ifaddr);

    memset(host, 0, NI_MAXHOST);
    gethostname(host, NI_MAXHOST);
    addr_list.insert(host);
    return 0;
}

int get_endpoints_cipher(int port, set<string>& endpoints_cp)
{
  set<string> ifaddrs;
  if(get_ifaddrs(ifaddrs))
    return -1;

  set<string>::iterator it;
  for (it= ifaddrs.begin(); it!= ifaddrs.end(); it++)
  {
    string endpoint= (*it);
    endpoint += ":";
    char port_str[10]= {0};
    sprintf(port_str, "%d", port);
    endpoint += port_str;

    int cipher= crc32(0, endpoint.c_str(), endpoint.length());
    char cipher_str[40]= {0};
    sprintf(cipher_str, "%x", cipher);
    endpoints_cp.insert(cipher_str);
  }
  return 0;
}
