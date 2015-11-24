/**********************************************************************
Copyright �2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

�	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
�	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

/************************* reduceRMQ Kernel ************************/
/**
 *  @brief    Compute RangeMinimumQuery operation on a given input range (startIndex, endIndex)
 *  @param    input input buffer stores input data
 *  @param    output output buffer stores minimum element for each block
 *  @param    sData shared memory buffer to perform work-level operation
*/
	                                                                    
__kernel 
void reduceRMQ( __global uint *input,
				__global uint *output,
				__local uint *sData
			  )                          
{
	unsigned int gid = get_global_id(0);
	unsigned int tid = get_local_id(0);
	unsigned int bid = get_group_id(0);

	// copy input data from global memory to local memory
	sData[tid] = input[gid];

	// reducing the block of local array to a single value 
	unsigned int min = work_group_reduce_min(sData[tid]);
	
	// writing result for this block to output buffer (global mem)
	if(tid == 0)
		output[bid] = min;

} 
