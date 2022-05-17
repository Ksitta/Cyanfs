#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <utility>
#include <vector>

#define min(a, b) ((a) < (b) ? (a) : (b))

#include "plugin/cyanfs/include/fs.h"
#include "plugin/cyanfs/include/lru.h"

static struct superblock sb __attribute__((aligned(4096)));
static Data databuf __attribute__((aligned(4096)));

static int fd = 0;

void create_disk(const std::string &path){
    fd = open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0777);
    assert(fd >= 0);
    sb.magic_number = MAGICNUM;
    sb.block_nums = (DISK_SIZE - sizeof(superblock) - sizeof(entry) * ENTRY_NUMS) / BSIZE;
    sb.data_start = (sizeof(superblock) + sizeof(entry) * ENTRY_NUMS) / BSIZE;
    sb.entry_size = ENTRY_NUMS;
    sb.entry_start = sizeof(superblock) / BSIZE;
    sb.error_check = 1;
    memset(sb.bitmap, 0, sizeof(sb.bitmap));
    write(fd, &sb, sizeof(sb));
    char *tmp = new char[DISK_SIZE];
    memset(tmp, 0, DISK_SIZE);
    write(fd, tmp, DISK_SIZE);
    delete[] tmp;
    close(fd);
}

void read_disk(int block_no){
    lseek(fd, block_no * BSIZE, SEEK_SET);
    int size = read(fd, &databuf, BSIZE);
    assert(size == BSIZE);
}


void write_disk(int block_no) {
    lseek(fd, block_no * BSIZE, SEEK_SET);
    write(fd, &databuf, BSIZE);
}

void setbitmap(entry * ent){
    if(ent->fsize == 0){
        return;
    }
    i64 pos = ent->block_start;
    while(pos != ent->last_block){
        int i = (pos - sb.data_start) / 8;
        int j = (pos - sb.data_start) % 8;
        sb.bitmap[i] |= (1 << j);
        read_disk(pos);
        pos = databuf.next;
    }
    if(pos != -1){
        int i = (pos - sb.data_start) / 8;
        int j = (pos - sb.data_start) % 8;
        sb.bitmap[i] |= (1 << j);
    }
}

void recover(){
    memset(sb.bitmap, 0, sizeof(sb.bitmap));
    for(int i = 0; i < sb.entry_size; i++){
        if(sb.entries[i].used){
            setbitmap(&sb.entries[i]);
        }
    }
}

void init(const std::string &path, bool format) {
    fd = open(path.c_str(), O_RDWR | O_NOATIME | O_DIRECT, 0777);
    if(fd == -1){
        create_disk(path);
        fd = open(path.c_str(), O_RDWR | O_NOATIME | O_DIRECT, 0777);
    }
    assert(fd >= 0);
    read(fd, &sb, sizeof(superblock));
    if(sb.magic_number != MAGICNUM || format){
        sb.magic_number = MAGICNUM;
        sb.block_nums = (DISK_SIZE - sizeof(superblock) - sizeof(entry) * ENTRY_NUMS) / BSIZE;
        sb.data_start = (sizeof(superblock) + sizeof(entry) * ENTRY_NUMS) / BSIZE;
        sb.entry_size = ENTRY_NUMS;
        sb.entry_start = sizeof(superblock) / BSIZE;
        sb.error_check = 1;
        memset(sb.bitmap, 0, sizeof(sb.bitmap));
        memset(sb.entries, 0, sizeof(sb.entries));
        lseek(fd, 0, SEEK_SET);
        write(fd, &sb, sizeof(sb));
    }
    if(sb.error_check == 0){
        recover();
    }
    lseek(fd, 0, SEEK_SET);
    sb.error_check = 0;
    write(fd, &sb, ERROR_CHECK_WRITE);
}


void write_entry(int pos) {
    lseek(fd, ENTRY_POS + (pos / ENTRY_PER_BLOCK) * BSIZE, SEEK_SET);
    int size = write(fd, &sb.entries[pos - (pos % ENTRY_PER_BLOCK)], BSIZE);
    assert(size == BSIZE);
}

void sync_fs(){
    fsync(fd);
}

i64 find_block(int start = 0) {
    int fstart = (start - sb.data_start) / 8;
    if(fstart < 0) {
        fstart = 0;
    }
    for (int i = fstart; i < BITMAP_SIZE; i++) {
        if (sb.bitmap[i] != 0xff) {
            for (int j = 0; j < 8; j++) {
                if ((sb.bitmap[i] & (1 << j)) == 0) {
                    sb.bitmap[i] |= (1 << j);
                    return i * 8 + j + sb.data_start;
                }
            }
        }
    }
    return -1;
}

int append(MemoryEntry *p_mentry) {
    entry *p_entry = &(sb.entries[p_mentry->pos]);
    i64 next_pos = p_entry->block_start;
    if (next_pos == -1) {
        i64 pos = find_block(p_entry->last_block);
        assert(pos != p_entry->last_block);
        p_entry->block_start = pos;
        p_entry->last_block = pos;
        p_mentry->cur_block = pos;
        write_entry(p_mentry->pos);
        return pos;
    }
    i64 where = p_entry->last_block;
    read_disk(where);
    i64 pos = find_block(p_entry->last_block);
    assert(pos != p_entry->last_block);
    databuf.next = pos;
    p_entry->last_block = pos;
    if (p_mentry->cur_block == -1) {
        p_mentry->cur_block = p_entry->last_block;
    }
    write_disk(where);
    write_entry(p_mentry->pos);
    return pos;
}

int find_entry(){
    for (int i = 0; i < sb.entry_size; i++){
        if (!sb.entries[i].used){
            return i;
        }
    }
    return -1;
}

MemoryEntry *look_up(const char *name){
    for (int i = 0; i < sb.entry_size; i++) {
        entry *cur = &sb.entries[i];
        if (cur->used && strcmp(cur->name, name) == 0){
            MemoryEntry *ret = new MemoryEntry();
            ret->pos = i;
            ret->offset = 0;
            ret->cur_block = cur->block_start;
            return ret;
        }
    }
    return NULL;
}

MemoryEntry *create(const char *name) {
    MemoryEntry *res = look_up(name);
    if (res != NULL)
    {
        return res;
    }
    auto pos = find_entry();
    if (pos == -1)
    {
        return NULL;
    }
    entry *cur = &(sb.entries[pos]);
    strcpy(cur->name, name);
    cur->block_start = -1;
    cur->used = 1;
    cur->fsize = 0;
    cur->last_block = -1;
    write_entry(pos);
    res = new MemoryEntry();
    res->pos = pos;
    res->offset = 0;
    res->cur_block = -1;
    return res;
}

int write(MemoryEntry *ment, const char *buffer, int len){
    entry *ent = &(sb.entries[ment->pos]);
    int p = 0;
    while (p < len) {
        int current_from = ent->fsize % INODE_BUFFER_SIZE;
        if (current_from == 0){
            append(ment);
        } else {
            read_disk(ent->last_block);
        }
        int write_size = min(INODE_BUFFER_SIZE - current_from, len - p);
        memcpy(databuf.buf + current_from, buffer + p, write_size);
        p += write_size;
        ent->fsize += write_size;
        write_disk(ent->last_block);
    }
    write_entry(ment->pos);
    return len;
}

bool eof(MemoryEntry *ment){
    if (ment->offset == sb.entries[ment->pos].fsize)
    {
        return true;
    }
    return false;
}

int read(MemoryEntry *ment, char *buffer, int len){
    entry *ent = &(sb.entries[ment->pos]);
    int fsize = ent->fsize;
    int p = 0;
    int current = ment->offset;
    while (p < len && current < fsize){
        int current_from = current % INODE_BUFFER_SIZE;
        read_disk(ment->cur_block);
        int read_size = min(min(INODE_BUFFER_SIZE - current_from, len - p),
                            fsize - current);
        memcpy(buffer + p, databuf.buf + current_from, read_size);
        p += read_size;
        current += read_size;
        ment->offset += read_size;
        if (current % INODE_BUFFER_SIZE == 0)
        {
            ment->cur_block = databuf.next;
        }
    }
    return p;
}

int seek(MemoryEntry *ment, u64 offset, int from){
    entry *ent = &(sb.entries[ment->pos]);
    if (from == SEEK_C) {
        offset += ment->offset;
    }
    if (offset > ent->fsize) {
        offset = ent->fsize;
    }
    if (offset < ment->offset) {
        ment->offset = 0;
        ment->cur_block = ent->block_start;
    }
    while (ment->offset < offset){
        read_disk(ment->cur_block);
        int current_from = ment->offset % INODE_BUFFER_SIZE;
        int move_size = min(INODE_BUFFER_SIZE - current_from, offset - ment->offset);
        ment->offset += move_size;
        if (ment->offset % INODE_BUFFER_SIZE == 0){
            ment->cur_block = databuf.next;
        }
    }
    return ment->offset;
}

MemoryEntry *open(const char *name) {
    MemoryEntry *ret = look_up(name);
    return ret;
}

int close(MemoryEntry *p) {
    if (p != NULL) {
        delete p;
        return 0;
    }
    return -1;
}

void destroy() {
    sb.error_check = 1;
    lseek(fd, 0, SEEK_SET);
    write(fd, &sb, ERROR_CHECK_WRITE);
    close(fd);
}

u64 fsize(MemoryEntry *ent){
    return sb.entries[ent->pos].fsize;
}

std::vector<std::string> lsdir(){
    std::vector<std::string> ret;
    for (int i = 0; i < sb.entry_size; i++){
        if (sb.entries[i].used != 0) {
            ret.push_back(std::string(sb.entries[i].name));
        }
    }
    return ret;
}

bool remove_file(const char *filename) {
    MemoryEntry *mement = look_up(filename);
    if (mement == nullptr) {
        return false;
    }
    memset(&sb.entries[mement->pos], 0, sizeof(entry));
    write_entry(mement->pos);
    i64 block = sb.entries[mement->pos].block_start;
    while (block != sb.entries[mement->pos].last_block) {
        i64 pos = block - sb.data_start;
        sb.bitmap[pos / 8] &= ~(1 << (pos % 8));
        read_disk(block);
        block = databuf.next;
    }
    if (block != -1) {
        i64 pos = block - sb.data_start;
        sb.bitmap[pos / 8] &= ~(1 << (pos % 8));
    }
    delete mement;
    return true;
}

bool rename_file(const char *oldname, const char *newname) {
    MemoryEntry *mement = look_up(oldname);
    MemoryEntry *mementnew = look_up(newname);
    if (mement == nullptr) {
        return false;
    }
    if (mementnew) {
        delete mement;
        delete mementnew;
        return false;
    }
    strcpy(sb.entries[mement->pos].name, newname);
    write_entry(mement->pos);
    delete mement;
    return true;
}
