#include "fs.h"
#include "kv.h"
#include <stdio.h>
#include <string.h>

int main()
{
    create_disk();
    init();
    create("123");
    KVStore *kv = new KVStore("123");
    kv->put("123", "456");
    std::string a;
    bool ret = kv->get("123", a);
    printf("result %d\n", ret);
    printf("%s\n", a.c_str());
    delete kv;
    destroy();
    return 0;
}