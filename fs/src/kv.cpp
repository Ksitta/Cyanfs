#include <stdint.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "fs.h"
#include "kv.h"

KVStore::KVStore(const std::string &dir) : dir(dir) {
  init(dir);
  file = open("current");
  MemoryEntry *newfile = nullptr;
  if (file == nullptr) {
    file = open("new");
    if (file == nullptr) {
      file = create("current");
    } else {
      rename_file("new", "current");
    }
  }
//   } else {
//     newfile = open("new");
//     if (newfile) {
//       close(newfile);
//       remove_file("new");
//     }
//     newfile = create("new");
//   }
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
//   if(newfile){
//       savekv(newfile);
//   }
}

KVStore::~KVStore() {
  close(file);
  destroy();
}

int KVStore::size() const { return mp.size(); }

bool KVStore::get(const std::string &key, std::string &value) const {
  auto iter = mp.find(key);
  if (iter != mp.end()) {
    value = iter->second;
    return true;
  }
  return false;
}

bool KVStore::put(const std::string &key, const std::string &val) {
  int log_size[2] = {int(key.size()), int(val.size())};
  offset += 8 + log_size[0];
  write(file, (char *)log_size, 8);
  write(file, key.c_str(), log_size[0]);
  write(file, val.c_str(), log_size[1]);
  mp[key] = val;
  return true;
}

bool KVStore::remove(const std::string &key) {
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

std::vector<std::string> KVStore::list() const {
  std::vector<std::string> ret;
  for (const auto &each : mp) {
    ret.push_back(each.first);
  }
  return ret;
}