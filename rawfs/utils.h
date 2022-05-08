#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string>

int open_dev(int no);
int close_dev(int no);
int read_raw(struct buf * b);
int write_raw(struct buf * b);

#endif // UTILS_H