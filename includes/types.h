#ifndef SKULL_TYPES_H
#define SKULL_TYPES_H

int typename_to_int(const char* name);

#ifdef SKULL_TYPES_H_IMPLEMENTATION

int typename_to_int(const char* name) {
    if (!name) return 0;
    
    // Map common type names to specific integers
    if (strcmp(name, "int") == 0) return 1;
    if (strcmp(name, "char") == 0) return 2;
    if (strcmp(name, "bool") == 0) return 3;
    if (strcmp(name, "float") == 0) return 4;
    if (strcmp(name, "void") == 0) return 5;
    if (strcmp(name, "string") == 0) return 6;
    
    // For unknown types, use the old hashing method
    int t = 0;
    size_t len = strlen(name);
    for (unsigned int i = 0; i < len; i++) {
        t += name[i];
    }
    return t + 100; // Add offset to avoid conflicts with predefined types
}

#endif // SKULL_TYPES_H_IMPLEMENTATION
#endif // SKULL_TYPES_H