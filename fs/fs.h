#ifndef __FS_H__
#define __FS_H__

typedef unsigned long long int u64;
typedef long long int i64;

const u64 DISK_SIZE = 1 << 30;
const u64 BSIZE = 512;
const int BITMAP_SIZE = DISK_SIZE / BSIZE / 8;
const int ENTRY_NUMS = 512;

struct superblock{
    u64 block_nums;
    i64 entry_start;
    i64 data_start;
    u64 entry_size;
    char bigmap[BITMAP_SIZE];
    char pad[512 - (4 * sizeof(u64) + BITMAP_SIZE) % 512];
};

struct entry{
    char name[64 - 2 * sizeof(u64) - sizeof(int)];
    int used;
    i64 block_start;
    u64 fsize;
};

struct dinode{
    char pad[512 - sizeof(i64)];
    i64 next;
};

struct inode{
    bool dirty;
    i64 block_no;
    union{
        struct{
            char buf[512 - sizeof(i64)];
            i64 next;
        } data;
        char entry[512];
    };
};


const int ENTRY_PER_BLOCK = BSIZE / sizeof(entry);

#endif