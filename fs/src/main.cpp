#include "fs.h"
#include "kv.h"
#include <stdio.h>
#include <string.h>

int main()
{
    create_disk(".");
    init(".");
    MemoryEntry * mem = create("123");
    write(mem, "123456", 6);
    close(mem);
    auto names = lsdir();
    for(auto each : names){
        printf("%s\n", each.c_str());
    }
    remove_file("123");
    names = lsdir();
    for(auto each : names){
        printf("%s\n", each.c_str());
    }
    destroy();
    return 0;
}