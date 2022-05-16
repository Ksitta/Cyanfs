#include "fs.h"
#include "kv.h"
#include <stdio.h>
#include <string.h>

int main()
{
    create_disk(".");
    init(".");
    KVStore *kv = new KVStore();

    kv->put("123", "234");
    std::string val;
    kv->get("123", val);

    printf("%s\n", val.c_str());
    delete kv;

    val = "";

    kv = new KVStore();
    kv->get("123", val);
    printf("%s\n", val.c_str());
    delete kv;

    destroy();
    return 0;
}