#include "fs.h"
#include <stdio.h>

int main()
{
    create_disk();
    init();
    MemoryEntry *entry = create("hello");
    write(entry, "12345", 5);
    char s[10] = {0};
    read(entry, s, 5);
    printf("%s\n", s);
    close(entry);
    destroy();
    return 0;
}