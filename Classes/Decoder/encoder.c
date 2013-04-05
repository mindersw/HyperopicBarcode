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

#include "encoder.h"


// Three possible encodings for each digit
char L[11] =
	{ 0xd, 0x19, 0x13, 0x3d, 0x23, 0x31, 0x2f, 0x3b, 0x37, 0xb, 0 };
char G[11] =
	{ 0x27, 0x33, 0x1b, 0x21, 0x1d, 0x39, 0x5, 0x11, 0x9, 0x17, 0 };
char R[11] =
	{ 0x72, 0x66, 0x6c, 0x42, 0x5c, 0x4e, 0x50, 0x44, 0x48, 0x74, 0 };

char LG[21] = { 0xd, 0x19, 0x13, 0x3d, 0x23, 0x31, 0x2f, 0x3b, 0x37, 0xb,
	0x27, 0x33, 0x1b, 0x21, 0x1d, 0x39, 0x5, 0x11, 0x9, 0x17, 0
};


// Select L or G to encode the first EAN digit
char FIRST_DIGIT[11] = { 0x0, 0xb, 0xd, 0xe, 0x13, 0x19,
	0x1c, 0x15, 0x16, 0x1a, 0
};


void encode_bits(int width, float *black, float *white, float *values,
				 int *sections, int count, int bits)
{
	for (int i = 0; i < count; i++) {
		float *source = bits >> (count - i - 1) & 1 ? black : white;
		for (int x = sections[i]; x < sections[i + 1]; x++)
			values[x] = source[x];
	}
}


void encode_ean13(int width, float *black, float *white, float *values,
				  int *sections, char *digits)
{
	encode_bits(width, black, white, values, &sections[0], 3, 0x5);
	encode_bits(width, black, white, values, &sections[45], 5, 0xa);
	encode_bits(width, black, white, values, &sections[92], 3, 0x5);
	char first = FIRST_DIGIT[digits[0] % 48];
	for (int i = 0; i < 6; i++) {
		int l = digits[1 + i] % 48;
		if (l < 10 && (first >> (5 - i)) & 1)
			l += 10;
		encode_bits(width, black, white, values, &sections[3 + i * 7], 7,
					LG[l]);
		int r = digits[7 + i] % 48;
		encode_bits(width, black, white, values, &sections[50 + i * 7], 7,
					R[r]);
	}
}
