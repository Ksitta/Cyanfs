#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <stdio.h>
#include "file.h"
#include "printf.h"
#include "string.h"
#include "types.h"


enum LOG_COLOR {
	RED = 31,
	GREEN = 32,
	BLUE = 34,
	GRAY = 90,
	YELLOW = 93,
};

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define panic(fmt, ...)                                                \
	do {                                                               \
		int tid = 0;                                                   \
		printf("\x1b[%dm[%s %d] %s:%d: " fmt "\x1b[0m\n", RED,         \
		       "PANIC", tid, __FILE__, __LINE__, ##__VA_ARGS__);       \
		exit(1);                                                       \
	} while (0)


#endif // DEF_H
