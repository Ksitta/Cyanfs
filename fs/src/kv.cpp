#include "kv.h"
#include "fs.h"

KVStore::KVStore() {
  file = open("current");
  MemoryEntry *newfile = open("new");
  if (file && newfile) {
    close(newfile);
    remove_file("new");
    newfile = create("new");
  } else if (!file && newfile) {
    rename_file("new", "current");
    file = newfile;
    newfile = nullptr;
  } else if (file && !newfile) {
    newfile = create("new");
  } else {
    file = create("current");
  }
  offset = 0;
  while (1) {
    int len[2];
    std::string key, val;
    read(file, (char *)len, 8);
    if (eof(file)) {
      break;
    }
    key.resize(len[0]);
    int key_len = read(file, &key[0], len[0]);
    if (key_len != len[0]) {
      break;
    }
    if (len[1]) {
      val.resize(len[1]);
      int val_len = read(file, &val[0], len[1]);
      if (val_len != len[1]) {
        break;
      }
      mp[key] = val;
    } else {
      mp.erase(key);
    }
    offset += 8 + len[0] + len[1];
  }
  if (newfile) {
    savekv(newfile);
    close(file);
    remove("current");
    rename_file("new", "current");
    file = newfile;
    newfile = nullptr;
  }
}

KVStore::~KVStore() { close(file); }

int KVStore::size() const { return mp.size(); }

bool KVStore::get(const std::string &key, std::string &value) {
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

void KVStore::savekv(MemoryEntry *ment) {
  for (const auto &each : mp) {
    int log_size[2] = {int(each.first.size()), int(each.second.size())};
    write(ment, (char *)(log_size), 8);
    write(ment, each.first.c_str(), log_size[0]);
    write(ment, each.second.c_str(), log_size[1]);
  }
}