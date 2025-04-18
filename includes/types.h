#ifndef SKULL_TYPES_H
#define SKULL_TYPES_H

int typename_to_int(const char* name);

#ifdef SKULL_TYPES_H_IMPLEMENTATION

int typename_to_int(const char* name) {
    int t = 0;
    size_t len = strlen(name);

    for (unsigned int  i = 0; i < len; i++) {
        t += name[i];
    }

    return t;
}

#endif // SKULL_TYPES_H_IMPLEMENTATION
#endif // SKULL_TYPES_H