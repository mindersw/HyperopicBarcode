#include "boolean.h"


typedef struct {
	float error;
	int index;
	char digit;
} Change;


bool is_valid_barcode(char *digits);

float best_valid_barcode(const Change *changes, int max_changes,
						 char *digits);
