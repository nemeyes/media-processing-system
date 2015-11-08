#ifndef _DK_FILE_IO_H_
#define _DK_FILE_IO_H_

#if defined __linux__
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>

typedef void * HANDLE;
typedef void             *HINSTANCE;
typedef unsigned long     DWORD, *LPDWORD;
typedef DWORD             FILE_SIZE;

#define FALSE   0
#define TRUE    1
#define INFINITE UINT_MAX

#define FILE_BEGIN               SEEK_SET
#define INVALID_SET_FILE_POINTER (-1)
#define INVALID_HANDLE_VALUE     ((void *)(-1))

#else
#include <stdio.h>
#include <windows.h>
#endif

typedef unsigned long long  U64;
typedef unsigned int        U32;

inline U32 set_file_pointer(HANDLE input_file, U32 file_offset, U32 * move_file_pointer, U32 flag)
{
#if defined (WIN32)
	return SetFilePointer(input_file, file_offset, 0, flag);
#elif defined __linux || defined __APPLE_ || defined __MACOSX
	return fseek((FILE *)input_file, file_offset, flag);
#endif
}

inline U32 set_file_pointer64(HANDLE input_file, U64 file_offset, U64 * move_file_pointer, U32 flag)
{
#if defined (WIN32)
	return SetFilePointer(input_file, ((U32 *)&file_offset)[0], (PLONG)&((U32 *)&file_offset)[1], flag);
#elif defined __linux || defined __APPLE__ || defined __MACOSX
	return fseek((FILE *)input_file, (long int)file_offset, flag);
#endif
}

inline bool read_file(HANDLE input_file, void * buf, U32 bytes_to_read, U32 * bytes_read, void * operlapped)
{
#if defined (WIN32)
	ReadFile(input_file, buf, bytes_to_read, (LPDWORD)bytes_read, 0);
	return true;
#elif defined __linux || defined __APPLE__ || defined __MACOSX
	U32 num_bytes_read;
	num_bytes_read = fread(buf, bytes_to_read, 1, (FILE *)input_file);

	if (bytes_read)
	{
		*bytes_read = num_bytes_read;
	}
	return true;
#endif
}

inline void get_file_size(HANDLE input_file, DWORD * filesize)
{
#if defined (WIN32)
	LARGE_INTEGER file_size;

	if (input_file != INVALID_HANDLE_VALUE)
	{
		file_size.LowPart = GetFileSize(input_file, (LPDWORD)&file_size.HighPart);
		printf("[ Input Filesize] : %ld bytes\n", ((LONGLONG)file_size.HighPart << 32) + (LONGLONG)file_size.LowPart);

		if (filesize != 0) 
			*filesize = file_size.LowPart;
	}

#elif defined __linux || defined __APPLE__ || defined __MACOSX
	FILE_SIZE file_size;

	if (input_file != NULL)
	{
		nvSetFilePointer64(input_file, 0, NULL, SEEK_END);
		file_size = ftell((FILE *)input_file);
		nvSetFilePointer64(input_file, 0, NULL, SEEK_SET);
		printf("Input Filesize: %ld bytes\n", file_size);

		if (filesize != NULL) 
			*filesize = file_size;
	}

#endif
}

inline HANDLE open_file(const char * input_file)
{
	HANDLE input = NULL;

#if defined (WIN32)
	input = CreateFileA(input_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (input == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "nvOpenFile Failed to open \"%s\"\n", input_file);
		exit(EXIT_FAILURE);
	}
#elif defined __linux || defined __APPLE_ || defined __MACOSX
	input = fopen(input_file, "rb");

	if (input == NULL)
	{
		fprintf(stderr, "nvOpenFile Failed to open \"%s\"\n", input_file);
		exit(EXIT_FAILURE);
	}
#endif
	return input;
}

inline HANDLE open_file_write(const char * output_file)
{
	HANDLE output = NULL;

#if defined (WIN32)
	output = CreateFileA(output_file, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (output == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "nvOpenFileWrite Failed to open \"%s\"\n", output_file);
		exit(EXIT_FAILURE);
	}

#elif defined __linux || defined __APPLE_ || defined __MACOSX
	output = fopen(output_file, "wb+");

	if (output == NULL)
	{
		fprintf(stderr, "nvOpenFileWrite Failed to open \"%s\"\n", output);
		exit(EXIT_FAILURE);
	}

#endif
	return output;
}

inline void close_file(HANDLE file)
{
	if (file)
	{
#if defined (WIN32)
		CloseHandle(file);
#else
		fclose((FILE *)file);
#endif
	}
}

#endif
