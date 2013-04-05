----------------->>> HYPEROPIC BARCODE <<<-----------------
===========================================================

>>>>>>>> SUMMARY <<<<<<<<
This project aims to develop a first-class barcode scanner that can use the built-in iSight on modern iMacs and MacBooks, which suffer from a close-focus distance which is too far from the camera to provide a clear image of the barcode. Because of this farsightedness ("hyperopia"), existing solutions struggled or would not work at all.

This decoder uses a mathematical simulator developed by Johann C. Rocholl for 1st generation iPhones which suffered from a similar problem. (See his thesis and original source code, enclosed.) It was updated to work with iSights by Michael LaMorte, Connie LaMorte, and Conor Dearden. Final debugging was contributed by Gwynne Raskind.

Hyperopic Barcode is in use in products by Minder Softworks (www.mindersoftworks.com) and Bruji (www.bruji.com)



>>>>>>>> THEORY OF OPERATION <<<<<<<<
Video frames are passed to the decoder. If a barcode is thought to be found, the scanner line changes from red to orange and the digits are handed off to a method that scrapes Amazon for the lookup data. If no results are found, the scanner goes "tink" and the line goes back to red; if information is found, the scanner line momentarily changes to green and the scanner goes "ding".

Because of the way the simulator works, it is possible that video frames that do not contain a barcode may generate a false barcode. For this reason, all suspected barcodes are sent to Amazon to verify their validity.




>>>>>>>> IMPLEMENTING IN YOUR PROJECT <<<<<<<<
1) Add the Classes, Libraries, and Images folders to your project

2) In your target, add linker flags to all the libraries:
	-lopencv_core
	-lopencv_imgproc
	-lstdc++
	-lz
	
3) In your target, add the following to the Header Search Path (with quotes): "$(SRCROOT)/Libraries" and make recursive

4) In your target, add the following to the Library Search Path (with quotes): "$(SRCROOT)/Libraries" and make recursive




>>>>>>>> LICENSE <<<<<<<<

Hyperopic Barcode is released under the 3-clause BSD License as follows:
-----------------------------------------------------------------------------------------
(c) Copyright 2012 Conor Dearden & Michael LaMorte
Portions (c) Copyright 2008-2009 Johann C. Rocholl

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice, 
  	  this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice, 
  	  this list of conditions and the following disclaimer in the documentation 
  	  and/or other materials provided with the distribution.
      * The name of the copyright holders may not be used to endorse or promote products
      derived from this software without specific prior written permission.
  	
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT  
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR  
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,  
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------------------


Hyperopic Barcode relies on the OpenCV library, which is also released under the 3-clause BSD license as follows:
-----------------------------------------------------------------------------------------
By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                           License Agreement
                For Open Source Computer Vision Library

Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
Third party copyrights are property of their respective owners.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   * Redistribution's of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   * Redistribution's in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   * The name of the copyright holders may not be used to endorse or promote products
     derived from this software without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall the Intel Corporation or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
-----------------------------------------------------------------------------------------
