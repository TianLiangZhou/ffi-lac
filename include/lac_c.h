//
// Created by meshell on 2021/4/17.
//

#ifndef LAC_C_LAC_C_H
#define LAC_C_LAC_C_H

#ifdef __cplusplus
extern "C"
{
#endif


typedef void * CLac;

typedef struct {
  const char *words;
  const char *tags;
  const char *weight;
} LacResult;

extern CLac new_lac(const char *mode);

extern void free_lac(CLac);

extern void free_result(LacResult*);

extern LacResult * parse(const char *input, CLac);

#ifdef __cplusplus
}
#endif
#endif //LAC_C_LAC_C_H
