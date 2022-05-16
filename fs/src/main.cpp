#include "fs.h"
#include "kv.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

int main()
{
    init("raw1", true);
    auto file = create("1234");
    close(file);

    auto ls = lsdir();

    for(auto each : ls){
        std::cout << each << std::endl;
    }
    std::cout << "-----------\n";

    int ret = rename_file("1234", "123");

    printf("%d\n", ret);
    ls = lsdir();

    for(auto each : ls){
        std::cout << each << std::endl;
    }

    file = open("123");
    close(file);
    remove_file("123");

    ls = lsdir();
    std::cout << "new ----\n";
    for(auto each : ls){
        std::cout << each << std::endl;
    }

    // printf("%p\n", file);
    
    return 0;
}