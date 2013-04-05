#include "minmax.h"


int compare_int(const void *a_ptr, const void *b_ptr)
{
	int a = *(int *) a_ptr;
	int b = *(int *) b_ptr;
	return a - b;
}


int compare_unsigned_char(const void *a_ptr, const void *b_ptr)
{
	int a = *(unsigned char *) a_ptr;
	int b = *(unsigned char *) b_ptr;
	return a - b;
}


int compare_float(const void *a_ptr, const void *b_ptr)
{
	float a = *(float *) a_ptr;
	float b = *(float *) b_ptr;
	if (a > b)
		return 1;
	if (b > a)
		return -1;
	return 0;
}
