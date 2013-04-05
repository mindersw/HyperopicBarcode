#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dir8.h"

#define PI 3.14159265358979323846f

int X8[8] = {1, 1, 0, -1, -1, -1,  0,  1};
int Y8[8] = {0, 1, 1,  1,  0, -1, -1, -1};


int dir8_atan2(int y, int x)
{
  if (5741 * abs(x) >= 13860 * abs(y)) return x > 0 ? 0 : 4;
  if (5741 * abs(y) >= 13860 * abs(x)) return y > 0 ? 2 : 6;
  if (x > 0) return y > 0 ? 1 : 7;
  if (x < 0) return y > 0 ? 3 : 5;
  return 0;
}


bool dir8_similar(int d1, int d2)
{
  int difference = abs(d2 - d1) % 8;
  return difference <= 1 || difference == 7;
}


void dir8_test()
{
  int x, y, d;
  for (int c = 0; c < 8; c++) {
    if (!dir8_similar(c, c + 1))
      printf("dir8: %d and %d should be similar\n", c, c + 1);
    if (!dir8_similar(c, c - 1))
      printf("dir8: %d and %d should be similar\n", c, c - 1);
    if (!dir8_similar(c, c + 7))
      printf("dir8: %d and %d should be similar\n", c, c + 7);
    if (!dir8_similar(c, c - 7))
      printf("dir8: %d and %d should be similar\n", c, c - 7);
    x = X8[c];
	y = Y8[c];
    d = dir8_atan2(y, x);
    if (c != d)
      printf("dir8_atan2(%d, %d) should return %d, not %d\n", y, x, c, d);
  }
  for (int a = 0; a < 64; a++) {
    float angle = ((float) a + 0.5) / 32.0 * PI * 2;
    int x = (int) (1000.0 * cos(angle));
	int y = (int) (1000.0 * sin(angle));
    int c = (a + 2) / 4 % 8;
    int d = dir8_atan2(y, x);
    if (c != d)
      printf("dir8(%d, %d) should return %d, not %d\n", y, x, c, d);
  }
}
