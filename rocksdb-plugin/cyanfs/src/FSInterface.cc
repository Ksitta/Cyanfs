#include "plugin/cyanfs/include/FSInterface.h"
#include "plugin/cyanfs/include/FileInterface.h"
#include <mutex>
#include <string.h>
#include <stdio.h>

std::mutex mtx;

namespace ROCKSDB_NAMESPACE {

CyanFS::CyanFS(): FileSystemWrapper(FileSystem::Default()) {
    puts("CyanFS Created");
    init("raw", true);
}

CyanFS::~CyanFS() {
    destroy();
}

const char* CyanFS::Name() const {
    return "cyanfs";
}

IOStatus CyanFS::NewSequentialFile(const std::string& fname, const FileOptions& file_opts, std::unique_ptr<FSSequentialFile>* result, IODebugContext* dbg) {
    mtx.lock();
    printf("New Seq File %s\n", fname.c_str());
    MySequentialFile *seqFile = new MySequentialFile(fname.c_str());
    mtx.unlock();
    if(!seqFile->isValid()) {
        return IOStatus::NotFound("Sequential File Not Found");
    }
    result->reset(seqFile);
    return IOStatus::OK();
}

IOStatus CyanFS::NewRandomAccessFile(const std::string& fname, const FileOptions& file_opts, std::unique_ptr<FSRandomAccessFile>* result, IODebugContext* dbg) {
    mtx.lock();
    printf("New RA File %s\n", fname.c_str());
    MyRandomAccessFile *randFile = new MyRandomAccessFile(fname.c_str());
    mtx.unlock();
    if(!randFile->isValid()) {
        return IOStatus::NotFound("RA File Not Found");
    }
    result->reset(randFile);
    return IOStatus::OK();
}
    
IOStatus CyanFS::NewWritableFile(const std::string& fname, const FileOptions& file_opts, std::unique_ptr<FSWritableFile>* result, IODebugContext* dbg) {
    printf("New Write File %s\n", fname.c_str());
    mtx.lock();
    MyWriteableFile *writeFile = new MyWriteableFile(fname.c_str());
    mtx.unlock();
    if(!writeFile->isValid()) {
        return IOStatus::IOError("Invalid File, new writable");
    }
    result->reset(writeFile);
    return IOStatus::OK();
}
    
IOStatus CyanFS::ReuseWritableFile(const std::string& fname, const std::string& old_fname, const FileOptions& file_opts, std::unique_ptr<FSWritableFile>* result, IODebugContext* dbg) {
    printf("Reuse File old: %s new: %s\n", old_fname.c_str(), fname.c_str());
    if(FileExists(fname, IOOptions(), dbg) == IOStatus::NotFound()) {
        return IOStatus::NotFound("Reuse not found.");
    }
    char *name = new char[old_fname.size() + 1];
    strcpy(name, old_fname.c_str());
    printf("Reuse require lock\n");
    fflush(stdout);
    mtx.lock();
    printf("Reuse get lock\n");
    fflush(stdout);
    remove_file(name);
    printf("Reuse release lock\n");
    fflush(stdout);
    printf("Reuse released\n");
    fflush(stdout);
    delete[] name;
    
    MyWriteableFile *writeFile = new MyWriteableFile(fname.c_str());
    mtx.unlock();
    if(!writeFile->isValid()) {
        return IOStatus::IOError("Reuse IO Error");
    }
    result->reset(writeFile);
    return IOStatus::OK();
}
    
IOStatus CyanFS::FileExists(const std::string& fname, const IOOptions& options, IODebugContext* dbg) {
    printf("Query Exist %s\n", fname.c_str());
    std::vector<std::string> lsResult = lsdir();
    for(const std::string &name: lsResult) {
        if(name == fname)
            return IOStatus::OK();
    }
    return IOStatus::NotFound("File doesn't exists");
}
    
IOStatus CyanFS::DeleteFile(const std::string& fname, const IOOptions& options, IODebugContext* dbg) {
    printf("Delete %s\n", fname.c_str());
    if(FileExists(fname, options, dbg) == IOStatus::NotFound()) {
        return IOStatus::OK();
    }
    char *name = new char[fname.size() + 1];
    strcpy(name, fname.c_str());
    printf("Delete require lock\n");
    fflush(stdout);
    mtx.lock();
    printf("Delete get lock\n");
    fflush(stdout);
    remove_file(name);
    printf("Delete finished");
    fflush(stdout);
    delete[] name;
    printf("Delete release lock\n");
    fflush(stdout);
    mtx.unlock();
    printf("Delete released\n");
    fflush(stdout);
    return IOStatus::OK();
}
    
IOStatus CyanFS::GetFileSize(const std::string& fname, const IOOptions& options, uint64_t* file_size, IODebugContext* dbg) {
    printf("Query FileSize %s\n", fname.c_str());
    if(FileExists(fname, options, dbg) == IOStatus::NotFound()) {
        *file_size = 0;
        return IOStatus::OK();
    }
    printf("FileSize require lock\n");
    fflush(stdout);
    mtx.lock();
    printf("FileSize get lock\n");
    fflush(stdout);
    MyWriteableFile *writeFile = new MyWriteableFile(fname.c_str());
    *file_size = writeFile->GetFileSize(options, dbg);
    delete writeFile;
    printf("FileSize release lock\n");
    fflush(stdout);
    mtx.unlock();
    printf("FileSize released\n");
    fflush(stdout);
    return IOStatus::OK();
}

IOStatus CyanFS::RenameFile(const std::string& src, const std::string& target, const IOOptions& options, IODebugContext* dbg) {
    printf("Rename %s to %s\n", src.c_str(), target.c_str());
    DeleteFile(target, options, dbg);
    printf("Rename require lock\n");
    fflush(stdout);
    mtx.lock();
    printf("Rename get lock\n");
    fflush(stdout);
    //TODO: rename logic
    char *srcname = new char[src.size() + 1];
    strcpy(srcname, src.c_str());
    char *targetname = new char[target.size() + 1];
    strcpy(targetname, target.c_str());
    rename_file(srcname, targetname);
    delete[] srcname;
    delete[] targetname;
    printf("Rename release lock\n");
    fflush(stdout);
    mtx.unlock();
    printf("Rename released\n");
    fflush(stdout);
    std::vector<std::string> lsResult = lsdir();
    printf("ls----\n");
    for(auto &name: lsResult) {
        printf("----%s\n", name.c_str());
    }
    printf("--------\n");
    fflush(stdout);
    return IOStatus::OK();
}
    
}