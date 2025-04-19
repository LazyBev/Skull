#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "skull.h"

void print_usage(const char* prog_name) {
    fprintf(stderr, "Usage: %s [options] input_file.k\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o, --output FILE    Specify output executable name\n");
    fprintf(stderr, "  -k, --keep-files     Keep intermediate .asm and .o files\n");
    fprintf(stderr, "  -h, --help           Show this help message\n");
}

void create_output_directory_if_needed(const char* path) {
    char* path_copy = strdup(path);
    char* dir = dirname(path_copy);

    if (strcmp(dir, ".") != 0) {
        struct stat st = {0};
        if (stat(dir, &st) == -1) {
            if (mkdir(dir, 0755) != 0) {
                perror("mkdir");
                fprintf(stderr, "Error: Failed to create directory '%s'\n", dir);
                exit(1);
            }
        }
    }

    free(path_copy);
}

int main(int argc, char* argv[]) {
    bool keep_files = false;
    const char* input_filename = NULL;
    const char* output_filename = "main";

    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"keep-files", no_argument, 0, 'k'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "o:kh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                output_filename = optarg;
                create_output_directory_if_needed(output_filename);
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

    if (optind < argc) {
        input_filename = argv[optind];
    } else {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    const char* ext = strrchr(input_filename, '.');
    if (!ext || strcmp(ext, ".k") != 0) {
        fprintf(stderr, "Error: Input file '%s' must have .k extension\n", input_filename);
        print_usage(argv[0]);
        return 1;
    }

    if (access(input_filename, F_OK) != 0) {
        fprintf(stderr, "Error: Input file '%s' does not exist\n", input_filename);
        return 1;
    }

    skull_compile_file(input_filename, output_filename, keep_files);

    return 0;
}
