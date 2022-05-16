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

    auto ls = lsdir();

    for(auto each : ls){
        std::cout << each << std::endl;
    }
    std::cout << "-----------\n";

    ls = lsdir();

    for(auto each : ls){
        std::cout << each << std::endl;
    }
    char buf1[10] = {0};

    file = open("1234");
    int red_cnt = read(file, buf1, 3);
    write(file, "abc", 3);
    int a = 0;
    printf("file content %s %d\n", buf1, red_cnt);
    scanf("%d", &a);
    close(file);

    ls = lsdir();
    std::cout << "new ----\n";
    for(auto each : ls){
        std::cout << each << std::endl;
    }
    file = open("1234");
    char buf[10] = {0};
    read(file, buf, 3);
    // printf("%p\n", file);
    close(file);
    destroy();
    printf("%s\n", buf);
    return 0;
}