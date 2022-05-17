#pragma once

#include <rocksdb/file_system.h>

namespace ROCKSDB_NAMESPACE {

std::unique_ptr<ROCKSDB_NAMESPACE::FileSystem> NewCyanFileSystem();

}  // namespace ROCKSDB_NAMESPACE
