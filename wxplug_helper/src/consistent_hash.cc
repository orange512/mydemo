
#include "consistent_hash.h"


void ConsistentHash::InitHashRing() {

  ip_list_1 = {
    "10.56.86.86",
    "10.56.86.88",
    "10.56.86.87",
    "10.56.86.89",
    "10.56.87.11",
    "10.56.87.12",
    "10.56.87.13",
    "100.66.31.68",
    "100.66.24.151",
    "10.165.12.154",
    "10.165.14.149",
    "10.165.12.225",
    "10.165.3.3",
  };

  ip_list_2 = { 
    "10.51.215.76",
    "10.50.147.80",
    "10.50.147.81",
    "10.50.147.82",
    "10.50.147.83",
    "10.50.147.220",
    "10.50.147.223",
    "10.50.147.227",
    "10.51.211.233",
  };

  _InitHashRing(server_1, ip_list_1);
  _InitHashRing(server_2, ip_list_2);
}


void ConsistentHash::_InitHashRing(vector<Server> &server, const vector<string> ip_list) {
  server.clear();
  int n = ip_list.size();
  // dummy node
  server.push_back(Server(0, -1, ""));
  for (int i = 0; i < n; ++i) {
    int sid = i + 1;

    int weight = NGX_CHASH_VIRTUAL_NODE_NUMBER;
    for (int j = 0; j < weight; ++j) {
      int id = sid * 256 * 16 + j;
      uint32_t hash = ngx_murmur_hash2((unsigned char *) (&id), 4);
      server.push_back(Server(hash, i, ip_list[i]));
    }
  }
  sort(server.begin(), server.end());
  for (int i = 1; i < server.size(); ++i) {
    // printf("%d %u %s %d\n", server[i].real_node_index, server[i].hash, server[i].ip.c_str(), 1);
  }
  return;
}


void ConsistentHash::GetNodeSet(string open_id,vector<string> &v1,vector<string> &v2) {
  uint32_t hash = ngx_murmur_hash2((u_char*)open_id.c_str(), open_id.length());
  v1 = _GetNodeSet(server_1, hash);
  v2 = _GetNodeSet(server_2, hash);
  //
  // set<string> ans = _GetNodeSet(server_1, hash);
  // set<string> ans = _GetNodeSet(server_2, hash);
  return;
}

vector<string> ConsistentHash::_GetNodeSet(const vector<Server> &server, uint32_t hash) {
  vector<string> ret;
  int number = server.size() - 1;
  int index = ngx_http_upstream_chash_get_server_index(server, number, hash);

  for (int i = 0; i < 2; ++i) {
    int t_index = (index + i) % number;
    if (t_index == 0) t_index = number;
    // printf("cur+%d %d %u %d %s\n", i, t_index, hash, server[t_index].real_node_index, server[t_index].ip.c_str());
    ret.push_back(server[t_index].ip);
  }
  return ret;
}


uint32_t ConsistentHash::ngx_murmur_hash2(u_char *data, size_t len) {
  uint32_t  h, k;
  h = 0 ^ len;
  while (len >= 4) {
    k  = data[0];
    k |= data[1] << 8;
    k |= data[2] << 16;
    k |= data[3] << 24;
    k *= 0x5bd1e995;
    k ^= k >> 24;
    k *= 0x5bd1e995;
    h *= 0x5bd1e995;
    h ^= k;
    data += 4;
    len -= 4;
  }
  switch (len) {
    case 3:
      h ^= data[2] << 16;
    case 2:
      h ^= data[1] << 8;
    case 1:
      h ^= data[0];
      h *= 0x5bd1e995;
  }
  h ^= h >> 13;
  h *= 0x5bd1e995;
  h ^= h >> 15;
  return h;
}


uint32_t
ConsistentHash::ngx_http_upstream_chash_get_server_index(const vector<Server>& server, uint32_t n, uint32_t hash) {
  uint32_t  low, hight, mid;
  low = 1;
  hight = n;
  while (low < hight) {
	mid = (low + hight) >> 1;
	if (server[mid].hash == hash) {
	  return mid;
	} else if (server[mid].hash < hash) {
	  low = mid + 1;
	} else {
	  hight = mid;
	}
  }
  if (low == n && server[low].hash < hash) {
	return 1;
  }
  return low;
}

