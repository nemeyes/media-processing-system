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
 * @file <SobelFilterLuma.cl>
 *
 * @brief OpenCL kernels for Sobel filter
 *
 ********************************************************************************
 */

__constant sampler_t sampler =  CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
__kernel void SobelFilterLuma(__read_only image2d_t inputImage, __write_only image2d_t outputImage)
{
    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    float4 Gx = (float4)(0);
    float4 Gy = Gx;

    /* Read each texel component and calculate the filtered value using neighbouring texel components */
    if( x >= 1 && x < (width-1) && y >= 1 && y < height - 1)
    {
        float4 i00 = convert_float4(read_imageui(inputImage, sampler, (int2)(x-1,y-1)));
        float4 i10 = convert_float4(read_imageui(inputImage, sampler, (int2)(x,y-1)));
        float4 i20 = convert_float4(read_imageui(inputImage, sampler, (int2)(x+1,y-1)));
        float4 i01 = convert_float4(read_imageui(inputImage, sampler, (int2)(x-1,y)));
        float4 i11 = convert_float4(read_imageui(inputImage, sampler, (int2)(x,y)));
        float4 i21 = convert_float4(read_imageui(inputImage, sampler, (int2)(x+1,y)));
        float4 i02 = convert_float4(read_imageui(inputImage, sampler, (int2)(x-1,y+1)));
        float4 i12 = convert_float4(read_imageui(inputImage, sampler, (int2)(x,y+1)));
        float4 i22 = convert_float4(read_imageui(inputImage, sampler, (int2)(x+1,y+1)));

        Gx =   i00 + (float4)(2) * i10 + i20 - i02  - (float4)(2) * i12 - i22;

        Gy =   i00 - i20  + (float4)(2)*i01 - (float4)(2)*i21 + i02  -  i22;

        /* taking root of sums of squares of Gx and Gy */
        uint4 out_pix = convert_uint4(hypot(Gx, Gy)/(float4)(2));

        write_imageui(outputImage, (int2)(x, y), out_pix);
    }
}

__kernel void ConstantChroma(__write_only image2d_t imageOut)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    uint4 out_pix;
    out_pix.x = 128;
    out_pix.y = 128;
    out_pix.z = 128;
    out_pix.w = 0;

    write_imageui(imageOut, (int2)((x*2), y), out_pix);
    write_imageui(imageOut, (int2)((x*2)+1, y), out_pix);
}
