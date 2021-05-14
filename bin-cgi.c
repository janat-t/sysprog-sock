#include <stdio.h>

int main(void) {
    printf("Content-Type: text/html\r\n");
    printf("\r\n");
    printf("<title>echo</title><p>This document is dynamically generated!</p>\r\n");
    return 0;
}
