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


set( SAMPLE_NAME SimpleDX10 )
set( SOURCE_FILES SimpleDX10.cpp )
set( EXTRA_FILES SimpleDX10_Kernels.cl SimpleDX10.bmp SimpleDX10.fx)
set( EXTRA_LIBRARIES_MSVC dxgi d3d10 )

set( INSTALL_TO_PUBLIC 1 )
set( INSTALL_ON_WINDOWS 1 )
set( INSTALL_ON_LINUX 0 )

############################################################################

set(CMAKE_SUPPRESS_REGENERATION TRUE)
cmake_minimum_required( VERSION 2.8.0 )
project( ${SAMPLE_NAME} )

# DirectX compilation works only on MSVC
if( NOT MSVC )
    return ( )
endif( )

# Auto-select bitness based on platform
if( NOT BITNESS )
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(BITNESS 64)
    else()
        set(BITNESS 32)
    endif()
endif()

# Select bitness for non-msvc platform. Can be specified as -DBITNESS=32/64 at command-line
if( NOT MSVC )
    set(BITNESS ${BITNESS} CACHE STRING "Specify bitness")
    set_property(CACHE BITNESS PROPERTY STRINGS "64" "32")
endif()
# Unset OPENCL_LIBRARIES, so that corresponding arch specific libs are found when bitness is changed
unset(OPENCL_LIBRARIES CACHE)

if( BITNESS EQUAL 64 )
    set(BITNESS_SUFFIX x86_64)
elseif( BITNESS EQUAL 32 )
    set(BITNESS_SUFFIX x86)
else()
    message( FATAL_ERROR "Bitness specified is invalid" )
endif()


############################################################################
# Find OpenCL include and libs
find_path( OPENCL_INCLUDE_DIRS 
    NAMES OpenCL/cl.h CL/cl.h
    HINTS
        $ENV{AMDAPPSDKROOT}/include
        $ENV{OPENCL_ROOT}/include
    PATHS
        /usr/include 
        /usr/local/include
)
mark_as_advanced(OPENCL_INCLUDE_DIRS)

if( BITNESS EQUAL 64 )
	find_library( OPENCL_LIBRARIES
		NAMES OpenCL
		HINTS
            $ENV{AMDAPPSDKROOT}/lib
			$ENV{OPENCL_ROOT}/lib
            /usr/lib
            /usr/lib64
		PATH_SUFFIXES x86_64 x64
	)
else( )
	find_library( OPENCL_LIBRARIES
		NAMES OpenCL
		HINTS
            $ENV{AMDAPPSDKROOT}/lib
			$ENV{OPENCL_ROOT}/lib
            /usr/lib32
            /usr/lib
		PATH_SUFFIXES x86
	)
endif( )
mark_as_advanced( OPENCL_LIBRARIES )

if( OPENCL_INCLUDE_DIRS STREQUAL "" OR OPENCL_LIBRARIES STREQUAL "")
	message( FATAL_ERROR "Could not locate OpenCL include & libs" )
endif( )

############################################################################
# Find DX include and libs for visual studio < 2012

if( MSVC_VERSION LESS 1700 )

    find_path( DX_INCLUDE_DIRS
        NAMES DXGI.h
        HINTS
            $ENV{DXSDK_DIR}/include
    )
    mark_as_advanced(DX_INCLUDE_DIRS)

    if( BITNESS EQUAL 64 )
        find_path( DX_LIBRARY_PATH
            NAMES dxgi.lib
            HINTS
                $ENV{DXSDK_DIR}/Lib/
            PATH_SUFFIXES x64
        )
    else( )
        find_path( DX_LIBRARY_PATH
            NAMES dxgi.lib
            HINTS
                $ENV{DXSDK_DIR}/Lib/
            PATH_SUFFIXES x86
        )
    endif( )
    mark_as_advanced( DX_LIBRARY_PATH )

    if( DX_INCLUDE_DIRS STREQUAL "" OR DX_LIBRARY_PATH STREQUAL "")
        message( FATAL_ERROR "Could not locate Directx include & libs" )
    endif( )
endif( )
############################################################################
# Find FX compiler
    
	if(BITNESS EQUAL 64) 
		set(CMAKE_PROGRAM_PATH "$ENV{PROGRAMFILES}/Windows Kits/8.0/bin/x64" : $ENV{DXSDK_DIR}/Utilities/bin/x64/)
	else()
	    set(CMAKE_PROGRAM_PATH "$ENV{PROGRAMFILES}/Windows Kits/8.0/bin/x86" : $ENV{DXSDK_DIR}/Utilities/bin/x86/)
    endif()
	
    find_program( FXC
	 fxc
    )
	
	if(NOT FXC)
	message(SEND_ERROR "Cannot find fx compiler!")
    endif(NOT FXC)

	add_custom_command(OUTPUT ${SAMPLE_NAME}.fxo
					   COMMAND ${FXC} /T fx_4_0 /Fo ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}.fxo ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}.fx 
                       MAIN_DEPENDENCY ${SAMPLE_NAME}.fx
                       COMMENT "Effect-compile"
                       VERBATIM)
		 
############################################################################
set( COMPILER_FLAGS " " )
set( LINKER_FLAGS " " )
set( ADDITIONAL_LIBRARIES "" )

file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h" )
include_directories( ${OPENCL_INCLUDE_DIRS} ../../../../include/SDKUtil ../../../../include/  ${DX_INCLUDE_DIRS})
link_directories( ${DX_LIBRARY_PATH} )



add_executable( ${SAMPLE_NAME} ${SOURCE_FILES} ${INCLUDE_FILES} ${EXTRA_FILES})

add_dependencies(${SAMPLE_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}.fxo")

set (EXTRA_FILES "${EXTRA_FILES}" "${SAMPLE_NAME}.fxo")


# gcc/g++ specific compile options
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set( COMPILER_FLAGS "${COMPILER_FLAGS} -msse2 " )
    
    # Note: "rt" is not present on mingw
    if( UNIX )
        set( ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} "rt" )
    endif( )
    
    if( BITNESS EQUAL 32 )
        set( COMPILER_FLAGS "${COMPILER_FLAGS} -m32 " )
        set( LINKER_FLAGS "${LINKER_FLAGS} -m32 " )
    else( )
        set( COMPILER_FLAGS "${COMPILER_FLAGS} -m64 " )
        set( LINKER_FLAGS "${LINKER_FLAGS} -m64 " )
    endif( )
    
    set( COMPILER_FLAGS "${COMPILER_FLAGS} ${EXTRA_COMPILER_FLAGS_GXX} " )
    set( LINKER_FLAGS "${LINKER_FLAGS} ${EXTRA_LINKER_FLAGS_GXX} " )
    set( ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ${EXTRA_LIBRARIES_GXX} )
elseif( MSVC )

    # Samples can specify additional libs/flags using EXTRA* defines
	add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo" )
    set( COMPILER_FLAGS "${COMPILER_FLAGS} ${EXTRA_COMPILER_FLAGS_MSVC} " )
    set( LINKER_FLAGS "${LINKER_FLAGS} ${EXTRA_LINKER_FLAGS_MSVC} /SAFESEH:NO " )
    set( ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} ${EXTRA_LIBRARIES_MSVC} )
endif( )

set_target_properties( ${SAMPLE_NAME} PROPERTIES
                        COMPILE_FLAGS ${COMPILER_FLAGS}
                        LINK_FLAGS ${LINKER_FLAGS}
                     )
target_link_libraries( ${SAMPLE_NAME} ${OPENCL_LIBRARIES} ${ADDITIONAL_LIBRARIES} )

# Set output directory to bin
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BITNESS_SUFFIX})

# Copy extra files to binary directory
foreach( extra_file ${EXTRA_FILES} )
    add_custom_command(
        TARGET ${SAMPLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/${extra_file}  ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/${extra_file}  ./
        )
endforeach( extra_file )

add_custom_command(
						TARGET ${SAMPLE_NAME} POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy
						${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}.fxo ./
					  )		

# Group sample based on FOLDER_GROUP defined in parent folder
if( FOLDER_GROUP )
    set_target_properties(${SAMPLE_NAME} PROPERTIES FOLDER ${FOLDER_GROUP})
endif( )

