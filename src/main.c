#define SKULL_H_IMPLEMENTATION
#include "include/skull.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH 1024

int main(int argc, char** argv) {
    int compile_only = 0;
    int run_final = 0;
    int link_only = 0;
    char* input_file = NULL;
    char* output_file = NULL;
    char* asm_file = NULL;
    char* obj_file = NULL;
    char* final_output = NULL;
    int status;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [-c] [-r] [-l] [-o <output>]\n", argv[0]);
        return 1;
    }

    input_file = argv[1];
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
            fprintf(stderr, "Usage: %s <filename> [-c] [-r] [-l] [-o <output>]\n", argv[0]);
            return 1;
        }
    }

    if (compile_only && run_final) {
        fprintf(stderr, "Error: -c cannot be used with -r\n");
        return 1;
    }

    if (compile_only && link_only) {
        fprintf(stderr, "Warning: Unneeded: Using both -c and -l provides same output as not using -c and -l\n");
    }

    asm_file = malloc(MAX_PATH);
    if (!asm_file) {
        fprintf(stderr, "Memory allocation failed for asm_file\n");
        return 1;
    }
    size_t len = strnlen(input_file, MAX_PATH);
    if (len >= MAX_PATH) {
        fprintf(stderr, "Input filename too long\n");
        free(asm_file);
        return 1;
    }
    strncpy(asm_file, input_file, MAX_PATH - 5);
    asm_file[MAX_PATH - 5] = '\0';
    if (len >= 4 && strcmp(asm_file + len - 4, ".sku") == 0) {
        strncpy(asm_file + len - 4, ".asm", 5);
    } else {
        strncat(asm_file, ".asm", MAX_PATH - len - 1);
    }

    if (!link_only || (link_only && compile_only)) {
        skull_compile_file(input_file);
    }

    if (compile_only && !link_only) {
        printf("Compiled to: %s\n", asm_file);
        free(asm_file);
        return 0;
    }

    final_output = malloc(MAX_PATH);
    if (!final_output) {
        fprintf(stderr, "Memory allocation failed for final_output\n");
        free(asm_file);
        return 1;
    }
    if (output_file) {
        strncpy(final_output, output_file, MAX_PATH - 1);
        final_output[MAX_PATH - 1] = '\0';
    } else {
        strncpy(final_output, input_file, MAX_PATH - 1);
        final_output[MAX_PATH - 1] = '\0';
        len = strnlen(final_output, MAX_PATH);
        if (len >= 4 && strcmp(final_output + len - 4, ".sku") == 0) {
            final_output[len - 4] = '\0';
        }
    }

    obj_file = malloc(MAX_PATH);
    if (!obj_file) {
        fprintf(stderr, "Memory allocation failed for obj_file\n");
        free(asm_file);
        free(final_output);
        return 1;
    }
    strncpy(obj_file, asm_file, MAX_PATH - 5);
    obj_file[MAX_PATH - 5] = '\0';
    len = strnlen(obj_file, MAX_PATH);
    if (len >= 4) {
        strncpy(obj_file + len - 4, ".o", 3);
        obj_file[len - 1] = '\0';
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        free(asm_file);
        free(obj_file);
        free(final_output);
        return 1;
    }
    if (pid == 0) {
        execlp("fasm", "fasm", "-m", "elf64", asm_file, obj_file, NULL);
        fprintf(stderr, "execlp fasm failed: %s\n", strerror(errno));
        exit(1);
    }
    if (wait(&status) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "fasm failed\n");
        free(asm_file);
        free(obj_file);
        free(final_output);
        return 1;
    }

    if (link_only || run_final || (!compile_only && !run_final && !link_only) || (compile_only && link_only)) {
        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed: %s\n", strerror(errno));
            free(asm_file);
            free(obj_file);
            free(final_output);
            return 1;
        }
        if (pid == 0) {
            execlp("ld", "ld", obj_file, "-o", final_output, NULL);
            fprintf(stderr, "execlp ld failed: %s\n", strerror(errno));
            exit(1);
        }
        if (wait(&status) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "ld failed\n");
            free(asm_file);
            free(obj_file);
            free(final_output);
            return 1;
        }
    }

    if (run_final) {
        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed: %s\n", strerror(errno));
            free(asm_file);
            free(obj_file);
            free(final_output);
            return 1;
        }
        if (pid == 0) {
            execlp(final_output, final_output, NULL);
            fprintf(stderr, "execlp run failed: %s\n", strerror(errno));
            exit(1);
        }
        if (wait(&status) == -1) {
            fprintf(stderr, "Wait failed: %s\n", strerror(errno));
            free(asm_file);
            free(obj_file);
            free(final_output);
            return 1;
        }
    }

    free(asm_file);
    free(obj_file);
    free(final_output);
    return 0;
}
