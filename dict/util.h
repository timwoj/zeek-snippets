#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>

#include "zeek-config.h"
#include "siphash24.h"

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef int64 bro_int_t;
typedef uint64 bro_uint_t;

typedef unsigned char u_char;

inline size_t pad_size(size_t size)
	{
	// We emulate glibc here (values measured on Linux i386).
	// FIXME: We should better copy the portable value definitions from glibc.
	if ( size == 0 )
		return 0;	// glibc allocated 16 bytes anyway.

	const int pad = 8;
	if ( size < 12 )
		return 2 * pad;

	return ((size+3) / pad + 1) * pad;
	}

#define padded_sizeof(x) (pad_size(sizeof(x)))

inline void out_of_memory(const char* where)
	{
	fprintf(stderr, "out of memory in %s.\n", where);
	abort();
	}

inline void* safe_realloc(void* ptr, size_t size)
	{
	ptr = realloc(ptr, size);
	if ( size && ! ptr )
		out_of_memory("realloc");

	return ptr;
	}

inline void* safe_malloc(size_t size)
	{
	void* ptr = malloc(size);
	if ( ! ptr )
		out_of_memory("malloc");

	return ptr;
	}

extern bool hmac_key_set;
extern uint8 shared_hmac_md5_key[16];
extern bool siphash_key_set;
extern uint8 shared_siphash_key[SIPHASH_KEYLEN];

extern void hmac_md5(size_t size, const unsigned char* bytes,
			unsigned char digest[16]);

#endif
