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
