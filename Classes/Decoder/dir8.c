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

#include "dir8.h"

#define PI 3.14159265358979323846f

int X8[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
int Y8[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };


int dir8_atan2(int y, int x)
{
	if (5741 * abs(x) >= 13860 * abs(y))
		return x > 0 ? 0 : 4;
	if (5741 * abs(y) >= 13860 * abs(x))
		return y > 0 ? 2 : 6;
	if (x > 0)
		return y > 0 ? 1 : 7;
	if (x < 0)
		return y > 0 ? 3 : 5;
	return 0;
}


int dir8_similar(int d1, int d2)
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
			printf("dir8_atan2(%d, %d) should return %d, not %d\n", y, x,
				   c, d);
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
