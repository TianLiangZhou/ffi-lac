#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

int init(const char* model, const char* dict, int ret_size, int t2s, int just_seg);
void deinit();
const char *getResult();
void freeResult();
int seg(const char *in);
