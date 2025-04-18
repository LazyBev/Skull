#define SKULL_H_IMPLEMENTATION

#include "skull.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <input_filename> [output_filename]\n", argv[0]);
        return 1;
    }
    
    const char* input_filename = argv[1];
    const char* output_filename = NULL;
    
    // Check if output filename was provided
    if (argc >= 3) {
        output_filename = argv[2];
    }
    
    skull_compile_file(input_filename, output_filename);
    return 0;
}
