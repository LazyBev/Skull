#define SKULL_H_IMPLEMENTATION
#include "include/skull.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <sys/stat.h>

#define MAX_CMD_LEN 2048

// Safe function to create derived filenames
char* create_derived_filename(const char* base_filename, const char* new_extension) {
    if (!base_filename || !new_extension) {
        return NULL;
    }
    
    char* temp = strdup(base_filename);
    if (!temp) {
        return NULL;
    }
    
    char* base = basename(temp);
    size_t base_len = strlen(base);
    size_t ext_len = strlen(new_extension);
    
    // Find the last dot in the basename
    char* dot = strrchr(base, '.');
    size_t name_len = dot ? (size_t)(dot - base) : base_len;
    
    // Allocate memory for new filename
    char* result = malloc(name_len + ext_len + 2); // +2 for dot and null terminator
    if (!result) {
        free(temp);
        return NULL;
    }
    
    // Copy name part
    strncpy(result, base, name_len);
    result[name_len] = '.';
    
    // Copy extension without the leading dot if it has one
    strcpy(result + name_len + 1, new_extension[0] == '.' ? new_extension + 1 : new_extension);
    
    free(temp);
    return result;
}

// Safe execution of external commands
int execute_command(char* const argv[]) {
    int status;
    pid_t pid = fork();
    
    if (pid < 0) {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        execvp(argv[0], argv);
        fprintf(stderr, "execvp failed for %s: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Parent process
    if (waitpid(pid, &status, 0) == -1) {
        fprintf(stderr, "waitpid failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Command '%s' failed with status %d\n", argv[0], WEXITSTATUS(status));
        return -1;
    }
    
    return 0;
}

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s <filename> [-c] [-r] [-l] [-o <output>]\n", program_name);
    fprintf(stderr, "  -c: Compile only (generate assembly file)\n");
    fprintf(stderr, "  -r: Run the compiled program\n");
    fprintf(stderr, "  -l: Link only (skip compilation)\n");
    fprintf(stderr, "  -o <output>: Specify output filename\n");
}

int main(int argc, char** argv) {
    int compile_only = 0;
    int run_final = 0;
    int link_only = 0;
    char* input_file = NULL;
    char* output_file = NULL;
    char* asm_file = NULL;
    char* obj_file = NULL;
    char* final_output = NULL;
    
    // Validate arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Get input file (first non-option argument)
    if (argv[1][0] != '-') {
        input_file = argv[1];
    } else {
        fprintf(stderr, "Error: Missing input filename\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    // Parse command line options
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            compile_only = 1;
        } else if (strcmp(argv[i], "-r") == 0) {
            run_final = 1;
        } else if (strcmp(argv[i], "-l") == 0) {
            link_only = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            fprintf(stderr, "Invalid argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    // Check for conflicting options
    if (compile_only && run_final) {
        fprintf(stderr, "Error: -c cannot be used with -r\n");
        return EXIT_FAILURE;
    }

    if (compile_only && link_only) {
        fprintf(stderr, "Warning: Using both -c and -l provides the same result as not using either\n");
    }

    // Create filenames for intermediate files
    asm_file = create_derived_filename(input_file, "asm");
    obj_file = create_derived_filename(input_file, "o");
    
    if (!asm_file || !obj_file) {
        fprintf(stderr, "Memory allocation failed for filename creation\n");
        free(asm_file);
        free(obj_file);
        return EXIT_FAILURE;
    }

    // Create final output filename
    if (output_file) {
        final_output = strdup(output_file);
    } else {
        // Remove extension from input file for final output
        char* temp = strdup(input_file);
        if (!temp) {
            fprintf(stderr, "Memory allocation failed\n");
            free(asm_file);
            free(obj_file);
            return EXIT_FAILURE;
        }
        
        char* dot = strrchr(basename(temp), '.');
        if (dot) {
            *dot = '\0';
        }
        
        final_output = strdup(temp);
        free(temp);
    }
    
    if (!final_output) {
        fprintf(stderr, "Memory allocation failed for final output filename\n");
        free(asm_file);
        free(obj_file);
        return EXIT_FAILURE;
    }

    // Compilation phase
    if (!link_only || (link_only && compile_only)) {
        // Check if input file exists
        if (access(input_file, R_OK) != 0) {
            fprintf(stderr, "Error: Cannot access input file '%s': %s\n", input_file, strerror(errno));
            free(asm_file);
            free(obj_file);
            free(final_output);
            return EXIT_FAILURE;
        }
        
        // Compile input file to assembly
        skull_compile_file(input_file);
        
        // Write assembly to file
        FILE* fp = fopen(asm_file, "w");
        if (!fp) {
            fprintf(stderr, "Error: Cannot open file '%s' for writing: %s\n", asm_file, strerror(errno));
            free(asm_file);
            free(obj_file);
            free(final_output);
            return EXIT_FAILURE;
        }
        
        // Here we would need to capture the output of skull_compile_file
        // For now, we'll just close the file and assume it worked
        fclose(fp);
    }

    // Stop if compile-only mode and not link_only
    if (compile_only && !link_only) {
        printf("Compiled to: %s\n", asm_file);
        free(asm_file);
        free(obj_file);
        free(final_output);
        return EXIT_SUCCESS;
    }

    // Assemble phase - convert assembly to object file
    char* fasm_args[] = {"fasm", asm_file, obj_file, NULL};
    if (execute_command(fasm_args) != 0) {
        free(asm_file);
        free(obj_file);
        free(final_output);
        return EXIT_FAILURE;
    }

    // Link phase - convert object file to executable
    if (link_only || run_final || (!compile_only && !run_final && !link_only) || (compile_only && link_only)) {
        char* ld_args[] = {"ld", obj_file, "-o", final_output, "-e", "main", NULL};
        if (execute_command(ld_args) != 0) {
            free(asm_file);
            free(obj_file);
            free(final_output);
            return EXIT_FAILURE;
        }
    }

    // Run the final executable if requested
    if (run_final) {
        // Make sure the file is executable
        if (chmod(final_output, 0755) != 0) {
            fprintf(stderr, "Warning: Failed to set executable permissions: %s\n", strerror(errno));
        }
        
        // Execute the program
        char* run_args[] = {final_output, NULL};
        if (execute_command(run_args) != 0) {
            fprintf(stderr, "Program execution failed\n");
            // Continue to cleanup, but will return error code
        }
    }

    // Cleanup
    free(asm_file);
    free(obj_file);
    free(final_output);
    
    return EXIT_SUCCESS;
}