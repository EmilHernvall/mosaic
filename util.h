#ifndef UTIL_H__
#define UTIL_H__

void* xmalloc(size_t);
void* xrealloc(void*, size_t);
void free(void*);
char* xstrdup(const char*);

#endif
