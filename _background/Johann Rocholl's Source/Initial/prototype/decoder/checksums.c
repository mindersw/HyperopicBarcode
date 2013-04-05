#include <cv.h>
#include "encoder.h"


int ean13_first_digit(const char *digits)
{
	int mask = 0;
	for (int i = 1; i <= 6; i++) {
		mask = mask << 1;
		if (digits[i] >= 10)
			mask += 1;
	}
	for (int d = 0; d < 10; d++) {
		if (FIRST_DIGIT[d] == mask)
			return d;
	}
	return -1;
}


int ean13_checksum(const char *digits)
{
	int sum = 0;
	for (int i = 0; i < 12; i++) {
		int digit = digits[i] % 48;
		if (i % 2)
			digit *= 3;
		sum += digit;
	}
	return (10 - (sum % 10)) % 10;
}


int upc_checksum(const char *digits)
{
	int sum = 0;
	for (int i = 0; i < 11; i++) {
		int digit = digits[i] % 48;
		if (i % 2 == 0)
			digit *= 3;
		sum += digit;
	}
	return (10 - (sum % 10)) % 10;
}


void checksums_test()
{
	assert(ean13_checksum("4009700021946") == 6);
	assert(ean13_checksum("9783540412601") == 1);
	assert(ean13_checksum("4035532100412") == 2);
	assert(ean13_checksum("4052700009308") == 8);
	assert(upc_checksum("041631000588") == 8);
	assert(upc_checksum("036000291452") == 2);
	assert(upc_checksum("015700050545") == 5);
	assert(upc_checksum("740500962308") == 8);
}
