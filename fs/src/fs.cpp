#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <queue>
#include <utility>

#define min(a,b) ((a) < (b) ? (a) : (b))

#include "fs.h"
#include "lru.h"

const int CACHE_SIZE = 1024;

static struct superblock sb;
static int fd = 0;
static inode inodes[CACHE_SIZE];
static FindReplace lru(CACHE_SIZE);

void create_disk(){
    fd = open("./raw", O_CREAT | O_RDWR);
    sb.block_nums = (DISK_SIZE - sizeof(superblock) - sizeof(entry) * ENTRY_NUMS) / BSIZE;
    sb.data_start = (sizeof(superblock) + sizeof(entry) * ENTRY_NUMS) / BSIZE;
    sb.entry_size = ENTRY_NUMS;
    sb.entry_start = sizeof(superblock) / BSIZE;
    memset(sb.bitmap, 0, sizeof(sb.bitmap));
    write(fd, &sb, sizeof(sb));
    char *tmp = new char[1<<30];
    memset(tmp, 0, sizeof(char) * (1<<30));
    write(fd, tmp, sizeof(char) * (1<<30));
    delete[] tmp;
    close(fd);
}

void init(){
    fd = open("./raw", O_RDWR, O_DIRECT | O_NOATIME);
    read(fd, &sb, sizeof(superblock));
    memset(inodes, 0, sizeof(inode) * CACHE_SIZE);
}

void write_disk(inode *node){
    if(node->dirty == false){
        return ;
    }
    node->dirty = false;
    lseek(fd, node->block_no * BSIZE, SEEK_SET);
    write(fd, (void *)&(node->data), BSIZE);
}

int alloc_cache(){
    int pos = lru.find();
    while(inodes[pos].refcount != 0)
        pos = lru.find();
    if(inodes[pos].dirty){
        write_disk(&inodes[pos]);
    }
    return pos;
}

inode* read_disk(int block_no){
    for(int i=0;i<CACHE_SIZE;i++)
        if(inodes[i].block_no == block_no)
            return inodes + i;
    int pos = alloc_cache();
    fsync(fd);
    lseek(fd, block_no * BSIZE, SEEK_SET);
    read(fd, &inodes[pos].data, BSIZE);
    inodes[pos].block_no = block_no;
    inodes[pos].dirty = false;
    inodes[pos].refcount = 0;
    return &inodes[pos];
}

void mark_dirty(inode *node){
    node->dirty = true;
}

i64 find_block(){
    for(int i = 0; i < BITMAP_SIZE; i++){
        if(sb.bitmap[i] != 0xff){
            for(int j = 0; j < 8; j++){
                if((sb.bitmap[i] & (1 << j)) == 0){
                    sb.bitmap[i] |= (1 << j);
                    return i * 8 + j + sb.data_start;
                }
            }
        }
    }
    return -1;
}

int append(MemoryEntry* p_mentry){
    inode *p_inode = read_disk(p_mentry->inode_number);
    entry *p_entry = &(p_inode->entries[p_mentry->pos]);
    i64 next_pos = p_entry->block_start;
    i64 pos = next_pos;
    if(next_pos == -1){
        i64 pos = find_block();
        p_entry->block_start = pos;
        p_entry->last_block = pos;
        mark_dirty(p_inode);
        p_mentry->cur_block = pos;
        return pos;
    }
    inode * node;
    node = read_disk(p_entry->last_block);
    node->data.next = find_block();
    p_entry->last_block = node->data.next;
    if(p_mentry->cur_block == -1)
    {
        p_mentry->cur_block = p_entry->last_block;
    }
    mark_dirty(node);
    return node->data.next;
}

int find_entry_from_node(inode* node){
    entry* p_entry = node->entries;
    for(int i = 0; i < ENTRY_PER_BLOCK; i++){
        if(!p_entry[i].used){
            return i;
        }
    }
    return -1;
}

std::pair<inode*, int> find_entry(){
    for(int i = 0; i < sb.entry_size / ENTRY_PER_BLOCK; i++){
        inode * node = read_disk(sb.entry_start + i);
        int pos = find_entry_from_node(node);
        if(pos != -1){
            return std::make_pair(node, pos); 
        }
    }
    return std::make_pair(nullptr, -1);
}

MemoryEntry* look_up(char *name){
    for(int i=0; i<sb.entry_size / ENTRY_PER_BLOCK; i++) {
        inode *node = read_disk(sb.entry_start + i);
        entry *p_entry = node->entries;
        for(int j=0;j<ENTRY_PER_BLOCK;j++) {
            entry *cur = &p_entry[i];
            if(cur->used && strcmp(cur->name, name) == 0) {
                MemoryEntry* ret = new MemoryEntry();
                ret->inode_number = node->block_no;
                ret->pos = j;
                ret->offset = 0;
                ret->cur_block = cur->block_start;
                ret->last_block = cur->last_block;
                return ret;
            }
        }
    }
    return NULL;
}

MemoryEntry* create(char *name){
    MemoryEntry *res = look_up(name);
    if(res != NULL) {
        return res;
    }
    auto pos = find_entry();
    if(pos.second == -1) {
        return NULL;
    }

    mark_dirty(pos.first);
    entry* p_entry = (entry*)&(pos.first->data);
    entry* cur = p_entry + pos.second;
    strcpy(cur->name, name);
    cur->block_start = -1;
    cur->used = 1;
    cur->fsize = 0;
    cur->last_block = -1;

    res = new MemoryEntry();
    res->inode_number = pos.first->block_no;
    res->pos = pos.second;
    res->offset = 0;
    res->cur_block = -1;
    return res;
}

int write(MemoryEntry *ment, char *buffer, int len) {
    inode *inod = read_disk(ment->inode_number);
    inod->refcount += 1;
    entry *ent = &(inod->entries[ment->pos]);
    int p = 0;
    while(p < len) {
        int current_from = ent->fsize % INODE_BUFFER_SIZE;
        if(current_from == 0)
        {
            append(ment);
        }
        int write_size = min(INODE_BUFFER_SIZE - current_from, len - p);
        inode *block = read_disk(ent->last_block);
        memcpy(block->data.buf + current_from, buffer + p,  write_size);
        p += write_size;
        ent->fsize += write_size;
        mark_dirty(block);
    }
    mark_dirty(inod);
    inod->refcount -= 1;
    return len;
}

int read(MemoryEntry *ment, char *buffer, int len) {
    inode *inod = read_disk(ment->inode_number);
    entry *ent = &(inod->entries[ment->pos]);
    int fsize = ent->fsize;
    int p = 0;
    int current = ment->offset;
    while(p < len && current < fsize) {
        int current_from = current % INODE_BUFFER_SIZE;
        inode *cur_inode = read_disk(ment->cur_block);
        int read_size = min(min(INODE_BUFFER_SIZE - current_from, len - p), fsize - current);
        memcpy(buffer + p, &cur_inode->data + current_from, read_size);
        p += read_size;
        current += read_size;
        ment->offset += read_size;
        if(current % INODE_BUFFER_SIZE == 0) {
            ment->cur_block = cur_inode->data.next;
        }
    }
    return p;
}

int seek(MemoryEntry *ment, int offset) {
    inode *inod = read_disk(ment->inode_number);
    entry *ent = &(inod->entries[ment->pos]);
    if(offset > ent->fsize) {
        return -1;
    }
    if(offset < ment->offset) {
        ment->offset = 0;
        ment->cur_block = ent->block_start;
    } 
    while(ment->offset < offset) {
        inode *cur_inode = read_disk(ment->cur_block);
        int current_from = ment->offset % INODE_BUFFER_SIZE;
        int move_size = min(INODE_BUFFER_SIZE - current_from, offset - ment->offset);
        ment->offset += move_size;
        if(ment->offset % INODE_BUFFER_SIZE == 0) {
            ment->cur_block = cur_inode->data.next;
        }
    }
    return ment->offset;
}

MemoryEntry *open(char *name) {
    MemoryEntry* ret = look_up(name);
    return ret;
}

int close(MemoryEntry *p) {
    if(p != NULL) {
        delete p;
        return 0;
    }
    return -1;
}

void destroy() {
    for(int i=0;i<CACHE_SIZE;i++)
        if(inodes[i].dirty)
            write_disk(&inodes[i]);
    close(fd);
}

u64 fsize(MemoryEntry *ent){
    inode * node = read_disk(ent->inode_number);
    return node->entries[ent->pos].fsize;
}

void clear_cache(){
    for(int i=0;i<CACHE_SIZE;i++){
        if(inodes[i].dirty){
            write_disk(&inodes[i]);
        }
    }
}