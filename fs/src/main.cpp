#include "fs.h"
#include <stdio.h>
#include <string.h>

int main()
{
    create_disk();
    init();
    MemoryEntry *entry = create("hello");
    // write(entry, "12345", 5);
    for(int i = 0; i < 2000; i++){
        write(entry, "12345", 5);
    }
    char s[1000] = {0};
    read(entry, s, 5);
    printf("%s\n", s);
    close(entry);
    entry = open("hello");
    memset(s, 0, 6);
    read(entry, s, 6);
    printf("%s\n", s);
    seek(entry, 0);
    memset(s, 0, 6);
    read(entry, s, 10);
    printf("%s\n", s);
    close(entry);
    entry = create("world");
    close(entry);
    auto vec = lsdir();
    for(auto each : vec){
        printf("%s\n", each.c_str());
    }
    destroy();
    return 0;
}