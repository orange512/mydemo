#ifndef CONSISTENT_HASH_H_
#define CONSISTENT_HASH_H_


#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstring>
#include <algorithm>

#define NGX_CHASH_VIRTUAL_NODE_NUMBER       160
using namespace std;


class ConsistentHash {
  public:
    void InitHashRing();
    void GetNodeSet(string open_id,vector<string> &set_1,vector<string> &set_2);

  private:
    uint32_t ngx_murmur_hash2(u_char *data, size_t len);

    struct Server {
      string ip;
      uint32_t real_node_index;
      uint32_t hash;
      Server(uint32_t h, uint32_t i, string _ip) : hash(h), real_node_index(i), ip(_ip) {}

      bool operator<(const Server& rhs) const {
        return hash < rhs.hash;
      }
    };

    uint32_t ngx_http_upstream_chash_get_server_index(const vector<Server> &server, uint32_t n, uint32_t hash);
    vector<string> _GetNodeSet(const vector<Server> &server, uint32_t hash);
    void _InitHashRing(vector<Server> &server, const vector<string> ip_list);

    vector<Server> server_1;
    vector<Server> server_2;
    vector<string> ip_list_1;
    vector<string> ip_list_2;
};

#endif  // CONSISTENT_HASH_H_
