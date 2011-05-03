#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void* xmalloc(size_t n)
{
	void* ptr = malloc(n);
	if (ptr == NULL) {
		abort();
	}

	return ptr;
}

void* xrealloc(void* ptr, size_t n)
{
	ptr = realloc(ptr, n);
	if (ptr == NULL) {
		abort();
	}

	return ptr;
}

void xfree(void* ptr)
{
	if (ptr == NULL) {
		return;
	}

	free(ptr);
}

char* xstrdup(const char* s)
{
	char* copy = strdup(s);
	if (copy == NULL) {
		abort();
	}

	return copy;
}

