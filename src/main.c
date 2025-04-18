#define SKULL_H_IMPLEMENTATION

#include "include/skull.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    
    skull_compile_file(argv[1]);
    return 0;
}