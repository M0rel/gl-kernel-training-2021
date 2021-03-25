#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

#define NSEC_IN_SEC     1000000000LL
#define MAX_VAL    64

#define POW2(x)     (1LL << (x))

uint64_t get_time_diff(struct timespec t1, struct timespec t2)
{
	if (t1.tv_sec == t2.tv_sec)
		return (t1.tv_nsec - t2.tv_nsec);

	uint64_t tmp_nsec = (NSEC_IN_SEC + t1.tv_nsec) - t2.tv_nsec;

	uint64_t tmp_sec = tmp_nsec / NSEC_IN_SEC;
	tmp_sec += (t1.tv_sec - t2.tv_sec - 1);

	return (tmp_sec * NSEC_IN_SEC) + (tmp_nsec % NSEC_IN_SEC);
}

struct alloc_free_time {
	uint64_t alloc_time;
	uint64_t free_time;
};

enum mem_func {
	MALLOC = 0,
	CALLOC,
	ALLOCA
};

bool get_alloca_time(uint64_t size, struct alloc_free_time *t)
{
	uint8_t *tmp;
	struct timespec t1, t2;
	t->free_time = 0;

	clock_gettime(CLOCK_REALTIME, &t2);
	tmp = alloca(size * sizeof(*tmp));
	clock_gettime(CLOCK_REALTIME, &t1);
	t->alloc_time = get_time_diff(t1, t2);

	if (tmp == NULL)
		return false;

	return true;
}

bool get_alloc_time(uint64_t size, struct alloc_free_time *t, enum mem_func func)
{
	uint8_t *tmp;
	struct timespec t1, t2;

	clock_gettime(CLOCK_REALTIME, &t2);

	switch(func) {
		case MALLOC:
			tmp = malloc(size * sizeof(*tmp));
		break;
		case CALLOC:
			tmp = calloc(size, sizeof(*tmp));
		break;
		default:
			tmp = NULL;
		break;
	}

	clock_gettime(CLOCK_REALTIME, &t1);
	t->alloc_time = get_time_diff(t1, t2);

	if (tmp == NULL)
		return false;

	clock_gettime(CLOCK_REALTIME, &t2);
	free(tmp);
	clock_gettime(CLOCK_REALTIME, &t1);
	t->free_time = get_time_diff(t1, t2);

	return true;
}

void test(char *text, enum mem_func func, uint8_t val)
{
	struct alloc_free_time *t = malloc(sizeof(*t));
	uint64_t size;
	bool ret;

	printf("--------------------------------------------------\n");
	printf("                 %s              \n", text);
	printf("                size = (2^x)+%d           \n", val);
	printf("--------------------------------------------------\n");

	for (uint8_t i = 0; i < MAX_VAL; i++) {

		printf("Iter: %d\n", i);

		size = val ? POW2(i) + 1 : POW2(i);

		if (func == ALLOCA)
			ret = get_alloca_time(size, t);
		else
			ret = get_alloc_time(size, t, func);

		if (ret == false) {
			printf("Can't allocate:(\n");
			break;
		}

		printf("Alloc time = %lldns\t", t->alloc_time);
		printf("Free time = %lldns\n", t->free_time);
	}

	free(t);
}

int main(void)
{
	test("MALLOC", MALLOC, 0);
	test("CALLOC", CALLOC, 0);
	test("MALLOC", MALLOC, 1);
	test("CALLOC", CALLOC, 1);
	test("ALLOCA", ALLOCA, 0);

	return 0;
}
