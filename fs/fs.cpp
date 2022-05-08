#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <queue>

#include "fs.h"
#include "lru.h"

const int CACHE_SIZE = 1024;

static struct superblock sb;
static int fd = 0;
static inode inodes[CACHE_SIZE];
static FindReplace lru(CACHE_SIZE);

void create(){
    fd = open("./raw", O_CREAT);
    sb.block_nums = (DISK_SIZE - sizeof(superblock) - sizeof(entry) * ENTRY_NUMS) / BSIZE;
    sb.data_start = (sizeof(superblock) + sizeof(entry) * ENTRY_NUMS) / BSIZE;
    sb.entry_size = ENTRY_NUMS;
    sb.entry_start = sizeof(superblock) / BSIZE;
    memset(sb.bigmap, 0, sizeof(sb.bigmap));
    write(fd, &sb, sizeof(sb));
    close(fd);
}

void init(){
    fd = open("./raw", O_RDWR, O_DIRECT | O_NOATIME);
    read(fd, &sb, sizeof(superblock));
    memset(inodes, 0, sizeof(inode) * CACHE_SIZE);
}

int alloc_cache(){
    int pos = lru.find();
    if(inodes[pos].dirty){
        write_disk(&inodes[pos]);
    }
    return pos;
}

inode* read_disk(int block_no){
    int pos = alloc_cache();
    lseek(fd, block_no * BSIZE, SEEK_SET);
    read(fd, &inodes[pos].data, BSIZE);
    return &inodes[pos];
}

void write_disk(inode *node){
    node->dirty = false;
    lseek(fd, node->block_no * BSIZE, SEEK_SET);
    write(fd);
}

void mark_dirty(inode *node){
    node->dirty = true;
}

int append(entry * p_entry){
    i64 next_pos = p_entry->block_start;
    i64 pos = next_pos;
    if(next_pos == 0){
        i64 pos = find_block();
        p_entry->block_start = pos;
        return pos;
    }
    inode * node;
    do{
        node = read_disk(pos);
        pos = next_pos;
        next_pos = node->data.next;
    }while(next_pos);
    node->data.next = find_block();
    mark_dirty(node);
    return node->data.next;
}

int find_entry_from_node(inode* node){
    entry* p_entry = (entry *)node->entry;
    for(int i = 0; i < ENTRY_PER_BLOCK; i++){
        if(!p_entry[i].used){
            return i;
        }
    }
}

int find_entry(){
    for(int i = 0; i < sb.entry_size / ENTRY_PER_BLOCK; i++){
        inode * node = read_disk(sb.entry_start + i);
        int pos = find_entry_from_node(node);
        if(pos != -1){
            return pos + i * ENTRY_PER_BLOCK; 
        }
    }
    return -1;
}

i64 find_block(){
    for(int i = 0; i < BITMAP_SIZE; i++){
        if(sb.bigmap[i] != 0xff){
            for(int j = 0; j < 8; j++){
                if((sb.bigmap[i] & (1 << j)) == 0){
                    return i * 8 + j + sb.data_start;
                }
            }
        }
    }
}

entry *look_up(char *name){
    
}

int create(char *name){

}