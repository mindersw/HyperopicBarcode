#pragma once


int ean13_first_digit(const char *digits);

int ean13_checksum(const char *digits);

void checksums_test();
