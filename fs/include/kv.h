#ifndef __KV_H__
#define __KV_H__

#include <stdint.h>
#include <string>
#include <vector>

#include "fs.h"

struct KeyOffset {
  std::string key;
  int offset;
  int len;

  KeyOffset(const std::string &key, int offset, int len)
      : key(key), offset(offset), len(len) {}
  KeyOffset() {}
};

struct Node {
  Node *right, *down;
  KeyOffset keypair;
  Node(Node *right, Node *down, const std::string &key, int offset, int len)
      : right(right), down(down), keypair(key, offset, len) {}
  Node() : right(nullptr), down(nullptr) {}
};

class KVStore {
private:
  int entry_size;
  int offset;

public:
  Node *head;
  MemoryEntry* file;
  std::string dir;

  KVStore(const std::string &dir) : dir(dir) {
    file = open(dir.c_str());
    head = new Node();
    entry_size = 0;
    offset = 0;
    while (1) {
        int len[2];
        std::string key;
        read(file, (char*)len, 8);
        if (eof(file)) {
            break;
        }
        key.resize(len[0]);
        read(file, &key[0], len[0]);
        seek(file, len[1], SEEK_C);
        put_nolog(key, len[1]);
    }
  }

  ~KVStore() {
    Node *p = head;
    Node *q = p;
    while (q) {
      p = q;
      q = q->down;
      while (p) {
        Node *tmp = p;
        p = p->right;
        delete tmp;
      }
    }
    close(file);
  }

  void readfile(int off, int len, std::string &value) {
    seek(file, off, SEEK_S);
    value.resize(len);
    read(file, &value[0], len);
  }

  int size() const { return entry_size; }

  bool get(const std::string &key, std::string &value) {
    Node *pos = head;
    while (pos) {
      while (pos->right && pos->right->keypair.key < key) {
        pos = pos->right;
      }

      if (pos->right && pos->right->keypair.key == key) {
        int off = pos->right->keypair.offset;
        int len = pos->right->keypair.len;
        if (len == 0) {
          return false;
        }
        readfile(off, len, value);
        return true;
      }
      pos = pos->down;
    }
    return false;
  }

  void put_skiplist(const std::string &key, bool &cover, int val_size,
                    std::vector<Node *> &path) {
    if (cover) {
      Node *edit = path.back()->right;
      while (edit) {
        edit->keypair.offset = offset;
        edit->keypair.len = val_size;
        edit = edit->down;
      }
    } else {
      Node *down_node = nullptr;
      int height = 1;
      int path_size = path.size();
      long rand_num;
      drand48_data randBuffer;
      for (int i = 0; i < path_size; i++) {
        lrand48_r(&randBuffer, &rand_num);
        if (rand_num & 1) {
          height++;
        } else {
          break;
        }
      }
      while (height && path.size() > 0) {
        Node *insert = path.back();
        path.pop_back();
        insert->right =
            new Node(insert->right, down_node, key, offset, val_size);
        down_node = insert->right;
        --height;
      }
      if (height) {
        Node *oldHead = head;
        head = new Node();
        head->right = new Node(nullptr, down_node, key, offset, val_size);
        head->down = oldHead;
      }
    }
    offset += val_size;
  }

  bool put(const std::string &key, const std::string &val) {
    bool cover = false;
    std::vector<Node *> path;
    find_pos(key, path, cover);
    int log_size[2] = {int(key.size()), int(val.size())};
    offset += 8 + log_size[0];
    write(file, (char *)log_size, 8);
    write(file, key.c_str(), log_size[0]);
    write(file, val.c_str(), log_size[1]);
    put_skiplist(key, cover, log_size[1], path);
    return true;
  }

  bool remove(const std::string &key) {
    bool cover = false;
    std::vector<Node *> path;
    find_pos(key, path, cover);
    if (cover) {
      Node *edit = path.back()->right;
      if (edit->keypair.len == 0) {
        return false;
      }
      while (edit) {
        edit->keypair.offset = offset;
        edit->keypair.len = 0;
        edit = edit->down;
      }
      int log_size[2] = {int(key.size()), 0};
      write(file, (char *)log_size, 8);
      write(file, key.c_str(), log_size[0]);
      offset += 8 + log_size[0];
      return true;
    }
    return false;
  }

  void find_pos(const std::string &key, std::vector<Node *> &path,
                bool &cover) {
    Node *pos = head;
    while (pos) {
      while (pos->right && pos->right->keypair.key < key) {
        pos = pos->right;
      }
      path.push_back(pos);
      if (pos->right && pos->right->keypair.key == key) {
        cover = true;
        break;
      }
      pos = pos->down;
    }
  }

  void put_nolog(const std::string &key, int val_size) {
    bool cover = false;
    std::vector<Node *> path;
    find_pos(key, path, cover);
    offset += 8 + key.size();
    put_skiplist(key, cover, val_size, path);
  }

  std::vector<std::string> list() const {
    std::vector<std::string> ret;
    Node *p = head;
    while (p->down) {
      p = p->down;
    }
    p = p->right;
    while (p) {
      ret.push_back(p->keypair.key);
    }
    return ret;
  }
};

#endif
