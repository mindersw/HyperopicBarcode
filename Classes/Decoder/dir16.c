//	(c) Copyright 2008-2009 Johann C. Rocholl
//	Portions (c) Copyright 2012 Conor Dearden & Michael LaMorte
//
//  Redistribution and use in source and binary forms, with or without modification, 
//  are permitted provided that the following conditions are met:
//
//      * Redistributions of source code must retain the above copyright notice, 
//  	  this list of conditions and the following disclaimer.
//      * Redistributions in binary form must reproduce the above copyright notice, 
//  	  this list of conditions and the following disclaimer in the documentation 
//  	  and/or other materials provided with the distribution.
//      * The name of the copyright holders may not be used to endorse or promote products
//        derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
//  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT  
//  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR  
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
//  POSSIBILITY OF SUCH DAMAGE.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dir16.h"

#define PI 3.14159265358979323846f

int X16[16] = { 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1, 0, 1, 2, 2 };
int Y16[16] = { 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1 };


int dir16_atan2(int y, int x)
{
	if (6401 * abs(x) >= 32180 * abs(y))
		return x > 0 ? 0 : 8;	// -11.25 to 11.25 degrees
	if (6401 * abs(y) >= 32180 * abs(x))
		return y > 0 ? 4 : 12;	// 87.75 to 101.25 degrees
	int quadrant = y > 0 ? 1 : 4;
	if (x < 0)
		quadrant = y > 0 ? 2 : 3;
	if (1915 * abs(x) >= 2866 * abs(y))
		switch (quadrant) {		// 11.25 to 33.75 degrees
		case 1:
			return 1;
		case 2:
			return 7;
		case 3:
			return 9;
		case 4:
			return 15;
		}
	if (1915 * abs(y) >= 2866 * abs(x))
		switch (quadrant) {		// 56.25 to 78.75 degrees
		case 1:
			return 3;
		case 2:
			return 5;
		case 3:
			return 11;
		case 4:
			return 13;
		}
	switch (quadrant) {			// 33.75 to 56.25 degrees
	case 1:
		return 2;
	case 2:
		return 6;
	case 3:
		return 10;
	case 4:
		return 14;
	}
	return 0;
}


int dir16_similar(int d1, int d2)
{
	int difference = abs(d2 - d1) % 16;
	return difference <= 1 || difference == 15;
}


void dir16_test()
{
	int x, y, d;
	for (int c = 0; c < 16; c++) {
		if (!dir16_similar(c, c + 1))
			printf("dir16: %d and %d should be similar\n", c, c + 1);
		if (!dir16_similar(c, c - 1))
			printf("dir16: %d and %d should be similar\n", c, c - 1);
		if (!dir16_similar(c, c + 15))
			printf("dir16: %d and %d should be similar\n", c, c + 15);
		if (!dir16_similar(c, c - 15))
			printf("dir16: %d and %d should be similar\n", c, c - 15);
		x = X16[c];
		y = Y16[c];
		d = dir16_atan2(y, x);
		if (c != d)
			printf("dir16_atan2(%d, %d) should return %d, not %d\n", y, x,
				   c, d);
	}
	for (int a = 0; a < 64; a++) {
		float angle = ((float) a + 0.5) / 32.0 * PI * 2;
		int x = (int) (1000.0 * cos(angle));
		int y = (int) (1000.0 * sin(angle));
		int c = (a + 1) / 2 % 16;
		int d = dir16_atan2(y, x);
		if (c != d)
			printf("dir16(%d, %d) should return %d, not %d\n", y, x, c, d);
	}
}
