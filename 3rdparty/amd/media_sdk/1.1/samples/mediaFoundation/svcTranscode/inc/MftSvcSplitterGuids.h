/*******************************************************************************
 Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice, 
 this list of conditions and the following disclaimer in the 
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/
/**  
 ********************************************************************************
 * @file <MftSvcSplitterGuids.h>                          
 *                                       
 * @brief This file contains class definition for svc splitter
 *         
 ********************************************************************************
 */
#ifndef MFTSVCSPLITTERGUIDS_H_
#define MFTSVCSPLITTERGUIDS_H_

/*******************************************************************************
 * Include Header files                                                         *
 *******************************************************************************/

#include <initguid.h>

// {CB3DA739-F371-416C-9A79-86979B0E0247}
DEFINE_GUID(CLSID_SvcMFT,
                0xcb3da739, 0xf371, 0x416c, 0x9a, 0x79, 0x86, 0x97, 0x9b, 0xe, 0x2, 0x47);

// {2E6AC518-B419-45CE-82CD-731812B316E0}
DEFINE_GUID(CLSID_SVC_MFT_OUTPUTS_NUMBER_PROPERTY,
                0x2e6ac518, 0xb419, 0x45ce, 0x82, 0xcd, 0x73, 0x18, 0x12, 0xb3, 0x16, 0xe0);

// {5599F84E-32C8-4DFB-9485-5981501ACE30}
DEFINE_GUID(CLSID_SVC_MFT_OUTPUT_LAYERS_NUM_PROPERTY,
                0x5599f84e, 0x32c8, 0x4dfb, 0x94, 0x85, 0x59, 0x81, 0x50, 0x1a, 0xce, 0x30);
#endif//MFTSVCSPLITTERGUIDS_H_
