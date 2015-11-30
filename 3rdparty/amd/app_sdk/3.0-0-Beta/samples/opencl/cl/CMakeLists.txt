#################################################################################
# Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# •	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# •	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
#  other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#################################################################################

set( FOLDER_NAME cl )
set( SUBDIRECTORIES AtomicCounters
BasicDebug
BinarySearchDeviceSideEnqueue
BinomialOption
BinomialOptionMultiGPU
BitonicSort
BlackScholes
BlackScholesDP
BoxFilter
BoxFilterGL
BufferBandwidth
BufferImageInterop
BuiltInScan
CalcPie
DCT
DeviceEnqueueBFS
DeviceFission
DeviceFission11Ext
DwtHaar1D
DynamicOpenCLDetection
ExtractPrimes
FastWalshTransform
FineGrainSVM
FineGrainSVMCAS
FloydWarshall
FluidSimulation2D
GaussianNoiseGL
HelloWorld
Histogram
ImageBandwidth
ImageBinarization
ImageOverlap
KernelLaunch
KmeansAutoclustering
LUDecomposition
Mandelbrot
MatrixMulImage
MatrixMultiplication
MatrixTranspose
MemoryModel
MonteCarloAsian
MonteCarloAsianDP
MonteCarloAsianMultiGPU
NBody
PipeProducerConsumerKernels
PrefixSum
QuasiRandomSequence
RadixSort
RecursiveGaussian
RecursiveGaussian_ProgramScope
Reduction
RegionGrowingSegmentation
ScanLargeArrays
SimpleConvolution
SimpleDepthImage
SimpleGenericAddressSpace
SimpleGL
SimpleImage
SimpleMultiDevice
SimplePipe
SimpleSPIR
SobelFilter
StringSearch
SVMAtomicsBinaryTreeInsert
SVMBinaryTreeSearch
Template
TransferOverlap
URNG 
RangeMinimumQuery )
set( SUBDIRECTORIES_WIN SimpleDX9 SimpleDX10 SimpleDX11 OpenCLService )

############################################################################

set(CMAKE_SUPPRESS_REGENERATION TRUE)
cmake_minimum_required( VERSION 2.8.0 )
project( ${FOLDER_NAME} )

# Group samples by folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set( FOLDER_GROUP ${FOLDER_GROUP}/${FOLDER_NAME} )

if( MSVC)
	set(SUBDIRECTORIES  ${SUBDIRECTORIES} ${SUBDIRECTORIES_WIN})
endif()

set( SUBDIRECTORIES_WIN "")
foreach( subdir ${SUBDIRECTORIES} )
    add_subdirectory( ${subdir} )
endforeach( subdir )
