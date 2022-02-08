/*
   SipHash reference C implementation

   Copyright (c) 2012-2014 Jean-Philippe Aumasson
   <jeanphilippe.aumasson@gmail.com>
   Copyright (c) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to>

   To the extent possible under law, the author(s) have dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along
   with
   this software. If not, see
   <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* default: SipHash-2-4 */
#define cROUNDS 2
#define dROUNDS 4

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define U32TO8_LE(p, v)                                                        \
  (p)[0] = (uint8_t)((v));                                                     \
  (p)[1] = (uint8_t)((v) >> 8);                                                \
  (p)[2] = (uint8_t)((v) >> 16);                                               \
  (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                                                        \
  U32TO8_LE((p), (uint32_t)((v)));                                             \
  U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p)                                                           \
  (((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) |                          \
   ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) |                   \
   ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) |                   \
   ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56))

#define SIPROUND                                                               \
  do {                                                                         \
    v0 += v1;                                                                  \
    v1 = ROTL(v1, 13);                                                         \
    v1 ^= v0;                                                                  \
    v0 = ROTL(v0, 32);                                                         \
    v2 += v3;                                                                  \
    v3 = ROTL(v3, 16);                                                         \
    v3 ^= v2;                                                                  \
    v0 += v3;                                                                  \
    v3 = ROTL(v3, 21);                                                         \
    v3 ^= v0;                                                                  \
    v2 += v1;                                                                  \
    v1 = ROTL(v1, 17);                                                         \
    v1 ^= v2;                                                                  \
    v2 = ROTL(v2, 32);                                                         \
  } while (0)

#ifdef SIPHASHDEBUG
#define TRACE                                                                  \
  do {                                                                         \
    printf("(%3d) v0 %08x %08x\n", (int)inlen, (uint32_t)(v0 >> 32),           \
           (uint32_t)v0);                                                      \
    printf("(%3d) v1 %08x %08x\n", (int)inlen, (uint32_t)(v1 >> 32),           \
           (uint32_t)v1);                                                      \
    printf("(%3d) v2 %08x %08x\n", (int)inlen, (uint32_t)(v2 >> 32),           \
           (uint32_t)v2);                                                      \
    printf("(%3d) v3 %08x %08x\n", (int)inlen, (uint32_t)(v3 >> 32),           \
           (uint32_t)v3);                                                      \
  } while (0)
#else
#define TRACE
#endif

// [Bro] We turn this into an internal function. siphash.h defines a wrapper.
int _siphash(uint8_t *out, const uint8_t *in, uint64_t inlen, const uint8_t* k) {
  /* "somepseudorandomlygeneratedbytes" */
  uint64_t v0 = 0x736f6d6570736575ULL;
  uint64_t v1 = 0x646f72616e646f6dULL;
  uint64_t v2 = 0x6c7967656e657261ULL;
  uint64_t v3 = 0x7465646279746573ULL;
  uint64_t b;
  uint64_t k0 = U8TO64_LE(k);
  uint64_t k1 = U8TO64_LE(k + 8);
  uint64_t m;
  int i;
  const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
  const int left = inlen & 7;
  b = ((uint64_t)inlen) << 56;
  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

#ifdef DOUBLE
  v1 ^= 0xee;
#endif

  for (; in != end; in += 8) {
    m = U8TO64_LE(in);
    v3 ^= m;

    TRACE;
    for (i = 0; i < cROUNDS; ++i)
      SIPROUND;

    v0 ^= m;
  }

  switch (left) {
  case 7:
    b |= ((uint64_t)in[6]) << 48;
  case 6:
    b |= ((uint64_t)in[5]) << 40;
  case 5:
    b |= ((uint64_t)in[4]) << 32;
  case 4:
    b |= ((uint64_t)in[3]) << 24;
  case 3:
    b |= ((uint64_t)in[2]) << 16;
  case 2:
    b |= ((uint64_t)in[1]) << 8;
  case 1:
    b |= ((uint64_t)in[0]);
    break;
  case 0:
    break;
  }

  v3 ^= b;

  TRACE;
  for (i = 0; i < cROUNDS; ++i)
    SIPROUND;

  v0 ^= b;

#ifndef DOUBLE
  v2 ^= 0xff;
#else
  v2 ^= 0xee;
#endif

  TRACE;
  for (i = 0; i < dROUNDS; ++i)
    SIPROUND;

  b = v0 ^ v1 ^ v2 ^ v3;
  U64TO8_LE(out, b);

#ifdef DOUBLE
  v1 ^= 0xdd;

  TRACE;
  for (i = 0; i < dROUNDS; ++i)
    SIPROUND;

  b = v0 ^ v1 ^ v2 ^ v3;
  U64TO8_LE(out + 8, b);
#endif

  return 0;
}


#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

bool verify(const uint64_t* key)
	{
	const uint8_t kMaxSize = 64;
	uint8_t in[kMaxSize];  // empty string, 00, 00 01, ...

	// Known-good SipHash-2-4 output from D. Bernstein.
	const uint64_t kSipHashOutput[64] = {
		0x726FDB47DD0E0E31, 0x74F839C593DC67FD, 0x0D6C8009D9A94F5A,
		0x85676696D7FB7E2D, 0xCF2794E0277187B7, 0x18765564CD99A68D,
		0xCBC9466E58FEE3CE, 0xAB0200F58B01D137, 0x93F5F5799A932462,
		0x9E0082DF0BA9E4B0, 0x7A5DBBC594DDB9F3, 0xF4B32F46226BADA7,
		0x751E8FBC860EE5FB, 0x14EA5627C0843D90, 0xF723CA908E7AF2EE,
		0xA129CA6149BE45E5, 0x3F2ACC7F57C29BDB, 0x699AE9F52CBE4794,
		0x4BC1B3F0968DD39C, 0xBB6DC91DA77961BD, 0xBED65CF21AA2EE98,
		0xD0F2CBB02E3B67C7, 0x93536795E3A33E88, 0xA80C038CCD5CCEC8,
		0xB8AD50C6F649AF94, 0xBCE192DE8A85B8EA, 0x17D835B85BBB15F3,
		0x2F2E6163076BCFAD, 0xDE4DAAACA71DC9A5, 0xA6A2506687956571,
		0xAD87A3535C49EF28, 0x32D892FAD841C342, 0x7127512F72F27CCE,
		0xA7F32346F95978E3, 0x12E0B01ABB051238, 0x15E034D40FA197AE,
		0x314DFFBE0815A3B4, 0x027990F029623981, 0xCADCD4E59EF40C4D,
		0x9ABFD8766A33735C, 0x0E3EA96B5304A7D0, 0xAD0C42D6FC585992,
		0x187306C89BC215A9, 0xD4A60ABCF3792B95, 0xF935451DE4F21DF2,
		0xA9538F0419755787, 0xDB9ACDDFF56CA510, 0xD06C98CD5C0975EB,
		0xE612A3CB9ECBA951, 0xC766E62CFCADAF96, 0xEE64435A9752FE72,
		0xA192D576B245165A, 0x0A8787BF8ECB74B2, 0x81B3E73D20B49B6F,
		0x7FA8220BA3B2ECEA, 0x245731C13CA42499, 0xB78DBFAF3A8D83BD,
		0xEA1AD565322A1A0B, 0x60E61C23A3795013, 0x6606D7E446282B93,
		0x6CA4ECB15C5F91E1, 0x9F626DA15C9625F3, 0xE51B38608EF25F57,
		0x958A324CEB064572};

	for (uint8_t size = 0; size < kMaxSize; ++size)
		{
		in[size] = size;
		uint64_t hash;
		_siphash((uint8_t*)&hash, in, size, (const uint8_t*)key);
		if (hash != kSipHashOutput[size])
			{
			printf("Mismatch at length %d\n", size);
			return false;
			}
		}

	return true;
	}

int main(int argc, char** argv)
	{
	const uint64_t key[2] = {0x0706050403020100ULL, 0x0F0E0D0C0B0A0908ULL};
    uint8_t bytes[32];

    if ( ! verify(key) )
	    _exit(1);

    for ( int bc = 0; bc < 32; ++bc )
	    {
	    bytes[bc] = bc;

	    timeval start;
	    timeval end;
	    uint64_t digest;

	    gettimeofday(&start, NULL);
	    for ( int i = 0; i < 10000000; ++i )
		    _siphash((uint8_t*)&digest, bytes, bc+1, (const uint8_t*)key);
	    gettimeofday(&end, NULL);

	    double start_time = (double)start.tv_sec + ((double)start.tv_usec / 1000000);
	    double end_time = (double)end.tv_sec + ((double)end.tv_usec / 1000000);
	    printf("Time for %d bytes: %.6f\n", bc+1, end_time - start_time);
	    }
	}
