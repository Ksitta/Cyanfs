#include "utils.h"
#include "bio.h"
#include "fs.h"

int devs[100];

int open_dev(int no){
    assert(devs[no] == 0);
    devs[no] = open(("./raw_data_" + std::to_string(no)).c_str(), O_RDWR | O_CREAT, O_DIRECT | O_NOATIME);
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