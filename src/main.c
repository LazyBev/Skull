#define SKULL_H_IMPLEMENTATION
#include "skull.h"
#include <string.h>
#include <stdio.h>

void print_usage(const char* program_name) {
    printf("Usage: %s [options] <input_file>\n", program_name);
    printf("Options:\n");
    printf("  -o <output_file>    Specify output file name (default: 'main')\n");
    printf("  -k, --keep         Keep intermediate files (.asm and .o)\n");
    printf("  -h, --help         Display this help message\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        fprintf(stderr, "Error: No arguments provided\n");
        return 1;
    }

    const char* input_filename = NULL;
    const char* output_filename = "main";
    skull_config_t config = {0}; // Default: remove intermediate files

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_filename = argv[++i];
            } else {
                fprintf(stderr, "Error: Missing value for -o option\n");
                print_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--keep") == 0) {
            config.keep_intermediate_files = 1;
        } else if (input_filename == NULL) {
            input_filename = argv[i];
        } else {
            fprintf(stderr, "Error: Unexpected argument '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (input_filename == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    skull_compile_file(input_filename, output_filename, config);
    return 0;
}
