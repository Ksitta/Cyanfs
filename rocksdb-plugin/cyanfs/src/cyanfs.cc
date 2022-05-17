#include "plugin/cyanfs/include/FSInterface.h"
#include <rocksdb/utilities/object_registry.h>
#include "plugin/cyanfs/include/cyanfs.h"

namespace ROCKSDB_NAMESPACE {

#ifndef ROCKSDB_LITE

extern "C" FactoryFunc<FileSystem> cyanfs_reg;

FactoryFunc<FileSystem> cyanfs_reg =
    ObjectLibrary::Default()->AddFactory<FileSystem>(
        "cyanfs",
        [](const std::string& /* uri */, std::unique_ptr<FileSystem>* f,
           std::string* /* errmsg */) {
          *f = NewCyanFileSystem();
          return f->get();
        });

#endif  // ROCKSDB_LITE

std::unique_ptr<FileSystem>
NewCyanFileSystem() {
  return std::unique_ptr<FileSystem>(
      new CyanFS());
}

}