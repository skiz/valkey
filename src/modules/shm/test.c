#include <stdio.h>
#include <unistd.h>

int main() {
    char buffer[100];
    write(1, "Testing write interception\n", 27);
    read(0, buffer, 100);
    return 0;
}