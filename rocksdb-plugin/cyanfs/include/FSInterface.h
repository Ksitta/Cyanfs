#pragma once

#include "rocksdb/file_system.h"
#include "plugin/cyanfs/include/FSInterface.h"
#include "plugin/cyanfs/include/fs.h"

namespace ROCKSDB_NAMESPACE {

class CyanFS: public FileSystemWrapper {
public:
    CyanFS();
    ~CyanFS();

    const char* Name() const override;

    IOStatus NewSequentialFile(const std::string& fname,
                               const FileOptions& file_opts,
                               std::unique_ptr<FSSequentialFile>* result,
                               IODebugContext* dbg);

    IOStatus NewRandomAccessFile(
      const std::string& fname, const FileOptions& file_opts,
      std::unique_ptr<FSRandomAccessFile>* result,
      IODebugContext* dbg);
    
    IOStatus NewWritableFile(const std::string& fname,
                                   const FileOptions& file_opts,
                                   std::unique_ptr<FSWritableFile>* result,
                                   IODebugContext* dbg);
    
    IOStatus ReuseWritableFile(const std::string& fname,
                                     const std::string& old_fname,
                                     const FileOptions& file_opts,
                                     std::unique_ptr<FSWritableFile>* result,
                                     IODebugContext* dbg);
    
    IOStatus FileExists(const std::string& fname,
                              const IOOptions& options,
                              IODebugContext* dbg);
    
    IOStatus DeleteFile(const std::string& fname,
                              const IOOptions& options,
                              IODebugContext* dbg);
    
    IOStatus GetFileSize(const std::string& fname,
                               const IOOptions& options, uint64_t* file_size,
                               IODebugContext* dbg);

    IOStatus RenameFile(const std::string& src, const std::string& target,
                              const IOOptions& options,
                              IODebugContext* dbg);
};

}