#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int fd = open("./test", O_RDWR | O_CREAT, O_DIRECT | O_NOATIME);
    char a[512];
    char b[512] = "1234\0";
    write(fd, b, 512);
    printf("%s\n", b);
    lseek(fd, 0, SEEK_SET);
    int l = read(fd, a, 512);
    printf("%s\n", a);
    return 0;
}