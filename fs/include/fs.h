#ifndef __FS_H__
#define __FS_H__

#include <vector>
#include <string>

typedef unsigned long long int u64;
typedef long long int i64;
typedef unsigned char u8;

const u64 DISK_SIZE = 1 << 30;
const u64 BSIZE = 512;
const int BITMAP_SIZE = DISK_SIZE / BSIZE / 8;
const int ENTRY_NUMS = 512;

struct entry{
    char name[64 - 3 * sizeof(u64) - sizeof(int)];
    int used;
    i64 block_start;
    u64 fsize;
    i64 last_block;
};

struct superblock{
    u64 block_nums;
    i64 entry_start;
    i64 data_start;
    u64 entry_size;
    u8 bitmap[BITMAP_SIZE];
    char pad[512 - ((4 * sizeof(u64) + sizeof(bitmap)) % 512)];
    entry entries[ENTRY_NUMS];
};

const int ENTRY_POS = sizeof(superblock) - sizeof(superblock::entries);

// When adding a new filed into MemoryEntry, remember to initial it in open/create
struct MemoryEntry {
    int pos;       // entry num
    int offset;    // read pointer
    i64 cur_block;
    i64 last_block;
};

struct dinode{
    char pad[512 - sizeof(i64)];
    i64 next;
};

const int ENTRY_PER_BLOCK = BSIZE / sizeof(entry);

struct inode{
    bool dirty;
    i64 block_no;
    struct{
        char buf[512 - sizeof(i64)];
        i64 next;
    } data;
    int refcount = 0;
};

const int INODE_BUFFER_SIZE =  (512 - sizeof(i64));

enum{
    SEEK_S,
    SEEK_C
};

void create_disk();
void init();
MemoryEntry *create(const char*);
int write(MemoryEntry*, const char*, int);
int read(MemoryEntry*, char*, int);
MemoryEntry *open(const char*);
int close(MemoryEntry*);
void destroy();
int seek(MemoryEntry *, int, int);
std::vector<std::string> lsdir();
bool remove_file(const char*);
bool rename_file(const char *oldname, const char *newname);
bool eof(MemoryEntry *);

#endif