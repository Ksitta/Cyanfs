#include "fs.h"
#include "kv.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

int main()
{

    {   
        KVStore kv("raw", false);
        kv.put("123", "456");

        kv.put("1234", "4567");
    }


    {
        std::string a;
        KVStore kv("raw", false);
        kv.get("123", a);
        std::cout << a << std::endl;
        kv.get("1234", a);
        std::cout << a << std::endl;
    }



    return 0;
}