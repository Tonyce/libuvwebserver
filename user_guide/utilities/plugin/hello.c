#include <stdio.h>

#include "plugin.h"

void mfp_register(const char *name) {
    fprintf(stderr, "Registered plugin \"%s\"\n", name);
}

void initialize() {
    mfp_register("Hello World!");
}
