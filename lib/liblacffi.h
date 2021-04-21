#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
//
// Created by meshell on 2021/4/18.
//


typedef void * CLac;

typedef struct {
    const char *words;
    const char *tags;
    const char *weight;
} LacResult;

CLac new_lac(const char *mode);

void free_lac(CLac);

void free_result(LacResult*);

LacResult * parse(const char *input, CLac);
