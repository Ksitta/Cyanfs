#ifndef __KV_H__
#define __KV_H__

#include <stdint.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "fs.h"

class KVStore {
private:
  int offset;

public:
  MemoryEntry *file;
  std::string dir;
  std::unordered_map<std::string, std::string> mp;

  KVStore(const std::string &dir) : dir(dir) {
    file = open(dir.c_str());
    offset = 0;
    while (1) {
      int len[2];
      std::string key, val;
      read(file, (char *)len, 8);
      if (eof(file)) {
        break;
      }
      key.resize(len[0]);
      read(file, &key[0], len[0]);
      if (len[1]) {
        val.resize(len[1]);
        read(file, &val[0], len[1]);
        mp[key] = val;
      } else {
        mp.erase(key);
      }
      offset += 8 + len[0] + len[1];
    }
  }

  ~KVStore() { close(file); }

  void readfile(int off, int len, std::string &value) {
    seek(file, off, SEEK_S);
    value.resize(len);
    read(file, &value[0], len);
  }

  int size() const { return mp.size(); }

  bool get(const std::string &key, std::string &value) {
    auto iter = mp.find(key);
    if (iter != mp.end()) {
      value = iter->second;
      return true;
    }
    return false;
  }

  bool put(const std::string &key, const std::string &val) {
    int log_size[2] = {int(key.size()), int(val.size())};
    offset += 8 + log_size[0];
    write(file, (char *)log_size, 8);
    write(file, key.c_str(), log_size[0]);
    write(file, val.c_str(), log_size[1]);
    mp[key] = val;
    return true;
  }

  bool remove(const std::string &key) {
    auto iter = mp.find(key);
    if (iter != mp.end()) {
      int log_size[2] = {int(key.size()), 0};
      write(file, (char *)log_size, 8);
      write(file, key.c_str(), log_size[0]);
      offset += 8 + log_size[0];
      mp.erase(iter);
      return true;
    }
    return false;
  }

  std::vector<std::string> list() const {
    std::vector<std::string> ret;
    for (const auto &each : mp) {
      ret.push_back(each.first);
    }
    return ret;
  }
};

#endif
