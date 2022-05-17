#pragma once

#include "rocksdb/file_system.h"
#include "plugin/cyanfs/include/fs.h"
#include <set>

namespace ROCKSDB_NAMESPACE {

class MySequentialFile: public FSSequentialFile {
private:
    MemoryEntry *entry;
public:
    MySequentialFile(const char *name);
    ~MySequentialFile();
    IOStatus Read(size_t n, const IOOptions& options, Slice* result,
                        char* scratch, IODebugContext* dbg) override;
    IOStatus Skip(uint64_t n) override;
    bool isValid() const;
};

class MyRandomAccessFile: public FSRandomAccessFile {
private:
    MemoryEntry *entry;
public:
    MyRandomAccessFile(const char *name);
    ~MyRandomAccessFile();
    IOStatus Read(uint64_t offset, size_t n, const IOOptions& options,
                        Slice* result, char* scratch,
                        IODebugContext* dbg) const override;
    bool isValid() const;
};

class MyWriteableFile: public FSWritableFile {
private:
    std::string name;
    MemoryEntry *entry;
    bool isClosed;
    bool isSynced;
public:
    MyWriteableFile(const char *name);
    ~MyWriteableFile();
    IOStatus Append(const Slice& data, const IOOptions& options,
                          IODebugContext* dbg) override;
    IOStatus Close(const IOOptions& options, IODebugContext* dbg) override;
    IOStatus Flush(const IOOptions& options, IODebugContext* dbg) override;
    IOStatus Sync(const IOOptions& options,
                        IODebugContext* dbg) override;  // sync data
    
    uint64_t GetFileSize(const IOOptions& /*options*/,
                               IODebugContext* /*dbg*/) override;
    bool isValid() const;
};

bool isOpenedForWrite(const std::string &name);


}