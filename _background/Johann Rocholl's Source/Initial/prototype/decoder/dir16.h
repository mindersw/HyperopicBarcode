#pragma once

#include "boolean.h"

extern int X16[16];
extern int Y16[16];

int dir16_atan2(int y, int x);

bool dir16_similar(int d1, int d2);

void dir16_test();
