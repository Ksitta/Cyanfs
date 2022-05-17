#include "plugin/cyanfs/include/FileInterface.h"

namespace ROCKSDB_NAMESPACE {

static std::set<std::string> openedForWrite;

MySequentialFile::MySequentialFile(const char *name): FSSequentialFile() {
    entry = open(name);
}

MySequentialFile::~MySequentialFile() {
    close(entry);
}

IOStatus MySequentialFile::Read(size_t n, const IOOptions& options, Slice* result, char* scratch, IODebugContext* dbg) {
    int read_len = read(entry, scratch, n);
    *result = Slice(scratch, read_len);
    return IOStatus::OK();
}

IOStatus MySequentialFile::Skip(uint64_t n) {
    int new_offset = entry->offset + n;
    int res = seek(entry, new_offset, SEEK_S);
    return IOStatus::OK();
}

bool MySequentialFile::isValid() const {
    return entry != NULL;
}

MyRandomAccessFile::MyRandomAccessFile(const char *name): FSRandomAccessFile() {
    entry = open(name);
}

MyRandomAccessFile::~MyRandomAccessFile() {
    close(entry);
}

IOStatus MyRandomAccessFile::Read(uint64_t offset, size_t n, const IOOptions& options, Slice* result, char* scratch, IODebugContext* dbg) const {
    if(offset > fsize(entry)) {
        *result = Slice(scratch, 0);
        return IOStatus::OK();
    }
    int res = seek(entry, offset, SEEK_S);
    int read_len = read(entry, scratch, n);
    *result = Slice(scratch, read_len);
    return IOStatus::OK();
}

bool MyRandomAccessFile::isValid() const {
    return entry != NULL;
}

MyWriteableFile::MyWriteableFile(const char *name) {
    entry = open(name);
    if(entry == NULL) {
        printf("Not found, creating!\n");
        entry = create(name);
        if(entry == NULL)
            printf("Not found and can't create\n");
        fflush(stdout);
    }
    isClosed = false;
    isSynced = true;
    this->name = std::string(name);
    openedForWrite.insert(this->name);
}

MyWriteableFile::~MyWriteableFile() {
    if(!isClosed)
        close(entry);
    openedForWrite.erase(name);
}

IOStatus MyWriteableFile::Append(const Slice &data, const IOOptions &options, IODebugContext *dbg) {
    if(isClosed)
        return IOStatus::IOError(); //already closed
    isSynced = false;
    write(entry, data.data(), data.size());
    return IOStatus::OK();
}

IOStatus MyWriteableFile::Flush(const IOOptions& options, IODebugContext* dbg) {
    return IOStatus::OK();
}

IOStatus MyWriteableFile::Sync(const IOOptions& options, IODebugContext* dbg) {
    if(isSynced)
        return IOStatus::OK();
    isSynced = true;
    if(isClosed)
        return IOStatus::IOError();
    //TODO: Sync Interface
    return IOStatus::OK();
}

IOStatus MyWriteableFile::Close(const IOOptions& options, IODebugContext* dbg) {
    if(isClosed)
        return IOStatus::OK();
    close(entry);
    isClosed = true;
    isSynced = true;
    openedForWrite.erase(name);
    return IOStatus::OK();
}

uint64_t MyWriteableFile::GetFileSize(const IOOptions& options, IODebugContext* dbg) {
    if(isClosed)
        return -1;
    return fsize(entry);
}

bool MyWriteableFile::isValid() const {
    return entry != NULL;
}

bool isOpenedForWrite(const std::string &name) {
    return openedForWrite.count(name) == 0 ? false : true;
}

}