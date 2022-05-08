#include "utils.h"
#include "bio.h"
#include "fs.h"

int devs[100];
int nbitmap = FSSIZE / (BSIZE * 8) + 1;
int ninodeblocks = NINODES / IPB + 1;

int create_dev(int no){
    devs[no] = creat(("./raw_data_" + std::to_string(no)).c_str(), S_IRUSR | S_IWUSR);
    lseek(devs[no], BSIZE, SEEK_SET);
    int nmeta = 2 + ninodeblocks + nbitmap;
	int nblocks = FSSIZE - nmeta;
    struct superblock sb;
    sb.magic = FSMAGIC;
	sb.size = FSSIZE;
	sb.nblocks = nblocks;
	sb.ninodes = NINODES;
	sb.inodestart = 2;
	sb.bmapstart = 2 + ninodeblocks;
    return devs[no];
}

int open_dev(int no){
    assert(devs[no] == 0);
    devs[no] = open(("./raw_data_" + std::to_string(no)).c_str(), O_RDWR, O_DIRECT | O_NOATIME);
    return no;
}

int close_dev(int no){
    assert(devs[no] != 0);
    close(devs[no]);
    devs[no] = 0;
}

int read_raw(struct buf * b){
    lseek(devs[b->dev], b->blockno * BSIZE, SEEK_SET);
    return read(devs[b->dev], b->data, BSIZE);
}

int write_raw(struct buf * b){
    lseek(devs[b->dev], b->blockno * BSIZE, SEEK_SET);
    return write(devs[b->dev], b->data, BSIZE);
}