#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "vector.h"

void vector_init(vector *v)
{
	assert(v != NULL);

	v->data = NULL;
	v->size = 0;
	v->count = 0;
}

int vector_count(vector *v)
{
	assert(v != NULL);

	return v->count;
}

void vector_add(vector *v, void *e)
{
	assert(v != NULL);
	assert(e != NULL);

	if (v->size == 0) {
		v->size = 10;
		v->data = xmalloc(sizeof(void*) * v->size);
		memset(v->data, '\0', sizeof(void) * v->size);
	}

	// condition to increase v->data:
	// last slot exhausted
	if (v->size == v->count) {
		v->size *= 2;
		v->data = xrealloc(v->data, sizeof(void*) * v->size);
	}

	v->data[v->count] = e;
	v->count++;
}

void vector_set(vector *v, int index, void *e)
{
	assert(v != NULL);
	assert(e != NULL);
	assert(index >= 0 && index < v->count);

	v->data[index] = e;
}

void *vector_get(vector *v, int index)
{
	assert(v != NULL);
	assert(index >= 0 && index < v->count);

	return v->data[index];
}

void vector_delete(vector *v, int index)
{
	assert(v != NULL);
	assert(index >= 0 && index < v->count);

	v->data[index] = NULL;

	int i;
	for (i = index + 1; i < v->count; i++) {
		v->data[i-1] = v->data[i];
	}

	v->count--;
}

void vector_free(vector *v)
{
	assert(v != NULL);

	v->size = 0;
	v->count = 0;
	xfree(v->data);
}

#ifdef TEST
int main(void)
{
	vector v;
	vector_init(&v);

	vector_add(&v, "emil");
	vector_add(&v, "hannes");
	vector_add(&v, "lydia");
	vector_add(&v, "olle");
	vector_add(&v, "erik");

	int i;
	printf("first round:\n");
	for (i = 0; i < vector_count(&v); i++) {
		printf("%s\n", vector_get(&v, i));
	}

	vector_delete(&v, 1);
	vector_delete(&v, 3);

	printf("second round:\n");
	for (i = 0; i < vector_count(&v); i++) {
		printf("%s\n", vector_get(&v, i));
	}

	vector_free(&v);

	return 0;
}
#endif
