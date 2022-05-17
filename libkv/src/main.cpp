#include "fs.h"
#include "kv.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

int main()
{
    init("raw1", false);
    auto file = create("1234");
    close(file);
    char buf1[10000] = {0};
    file = open("1234");
    write(file, buf1, 10000);
    destroy();
    return 0;
}