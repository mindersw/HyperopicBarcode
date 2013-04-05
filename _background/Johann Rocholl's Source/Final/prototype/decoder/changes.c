#include "changes.h"
#include "checksums.h"
#include <string.h>
#include <stdio.h>

#define MAX_ERROR 1000000.0


bool is_valid_barcode(char *digits)
{
	if (digits[0] < 0 || digits[0] > 9)
		return false;
	return digits[12] == ean13_checksum(digits);
}


float best_valid_barcode(const Change *changes, int max_changes,
						 char *digits)
{
	if (is_valid_barcode(digits))
		return 0.0;
	float best_error = MAX_ERROR;
	char best_digits[13];
	memcpy(best_digits, digits, 13);
	char attempt[13];
	for (int i = 0; changes[i].index >= 0; i++) {
		memcpy(attempt, digits, 13);
		attempt[changes[i].index] = changes[i].digit;
		if (changes[i].index >= 2 && changes[i].index <= 6)
			attempt[0] = ean13_first_digit(attempt);
		float error = changes[i].error;
		if (max_changes > 1)
			error +=
				best_valid_barcode(&changes[i + 1], max_changes - 1,
								   attempt);
		else if (!is_valid_barcode(attempt))
			continue;
		if (error < best_error) {
			best_error = error;
			memcpy(best_digits, attempt, 13);
			// printf("best_error=%.6f\n", best_error);
		}
	}
	memcpy(digits, best_digits, 13);
	return best_error;
}
