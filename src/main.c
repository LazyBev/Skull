#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "skull.h"

void print_usage(const char* prog_name) {
    fprintf(stderr, "Usage: %s [options] input_file.k\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o, --output FILE    Specify output executable name\n");
    fprintf(stderr, "  -k, --keep-files     Keep intermediate .asm and .o files\n");
    fprintf(stderr, "  -h, --help           Show this help message\n");
}

int main(int argc, char* argv[]) {
    bool keep_files = false;
    const char* input_filename = NULL;
    const char* output_filename = NULL;

    // Define long options for getopt_long
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"keep-files", no_argument, 0, 'k'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Parse command-line arguments
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "o:kh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                output_filename = optarg;
                break;
            case 'k':
                keep_files = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Check for input file (non-option argument)
    if (optind < argc) {
        input_filename = argv[optind];
    } else {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    // Check if input file has .k extension
    const char* ext = strrchr(input_filename, '.');
    if (!ext || strcmp(ext, ".k") != 0) {
        fprintf(stderr, "Error: Input file '%s' must have .k extension\n", input_filename);
        print_usage(argv[0]);
        return 1;
    }

    // Validate input file
    if (access(input_filename, F_OK) != 0) {
        fprintf(stderr, "Error: Input file '%s' does not exist\n", input_filename);
        return 1;
    }

    // Call the compiler
    skull_compile_file(input_filename, output_filename, keep_files);

    return 0;
}