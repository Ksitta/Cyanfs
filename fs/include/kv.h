#ifndef __KV_H__
#define __KV_H__

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

struct MemoryEntry;

class KVStore {
private:
  int offset;
  MemoryEntry *file;
  std::string dir;
  std::unordered_map<std::string, std::string> mp;

public:
  KVStore(const std::string &dir);
  ~KVStore();
  int size() const;
  bool get(const std::string &key, std::string &value) const;
  bool put(const std::string &key, const std::string &val);
  bool remove(const std::string &key);
  std::vector<std::string> list() const;
};

#endif
