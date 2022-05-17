#ifndef __FS_H__
#define __FS_H__

#include <string>
#include <vector>

typedef unsigned long long int u64;
typedef long long int i64;
typedef unsigned char u8;

const u64 DISK_SIZE = 1 << 30;
const u64 BSIZE = 4096;
const int BITMAP_SIZE = DISK_SIZE / BSIZE / 8;
const int ENTRY_NUMS = 512;

const u64 MAGICNUM = 0x202205012;

struct entry
{
    char name[128 - 3 * sizeof(u64) - sizeof(int)];
    int used;
    i64 block_start;
    u64 fsize;
    i64 last_block;
};

struct superblock
{
    u64 magic_number;
    u64 block_nums;
    i64 entry_start;
    i64 data_start;
    u64 entry_size;
    u8 bitmap[BITMAP_SIZE];
    u64 error_check;
    char pad[BSIZE - ((6 * sizeof(u64) + sizeof(bitmap)) % BSIZE)];
    entry entries[ENTRY_NUMS];
};

const int ERROR_CHECK_WRITE = 6 * sizeof(u64) + sizeof(superblock::bitmap) + (BSIZE - ((6 * sizeof(u64) + sizeof(superblock::bitmap)) % BSIZE));

const int ENTRY_POS = sizeof(superblock) - sizeof(superblock::entries);

// When adding a new filed into MemoryEntry, remember to initial it in
// open/create
struct MemoryEntry{
    int pos;     // entry num
    int offset;  // read pointer
    i64 cur_block;
};

const int ENTRY_PER_BLOCK = BSIZE / sizeof(entry);

struct Data{
    char buf[BSIZE - sizeof(i64)];
    i64 next;
};

const int INODE_BUFFER_SIZE = (BSIZE - sizeof(i64));

enum
{
    SEEK_S,
    SEEK_C
};

void create_disk(const std::string &path);
void init(const std::string &path, bool format);
MemoryEntry *create(const char *);
int write(MemoryEntry *, const char *, int);
int read(MemoryEntry *, char *, int);
MemoryEntry *open(const char *);
int close(MemoryEntry *);
void destroy();
int seek(MemoryEntry *, u64, int);
std::vector<std::string> lsdir();
bool remove_file(const char *);
bool rename_file(const char *oldname, const char *newname);
bool eof(MemoryEntry *);
void sync_fs();
u64 fsize(MemoryEntry *ent);

#endif