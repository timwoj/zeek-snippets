#include <stdint.h>

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
	__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define _le64toh(x) ((uint64_t)(x))
#elif defined(_WIN32)
/* Windows is always little endian, unless you're on xbox360
   http://msdn.microsoft.com/en-us/library/b0084kay(v=vs.80).aspx */
#  define _le64toh(x) ((uint64_t)(x))
#elif defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
#  define _le64toh(x) OSSwapLittleToHostInt64(x)
#else

/* See: http://sourceforge.net/p/predef/wiki/Endianness/ */
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#    include <sys/endian.h>
#  else
#    include <endian.h>
#  endif
#  if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && \
	__BYTE_ORDER == __LITTLE_ENDIAN
#    define _le64toh(x) ((uint64_t)(x))
#  else
#    define _le64toh(x) le64toh(x)
#  endif

#endif


#define ROTATE(x, b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define HALF_ROUND(a,b,c,d,s,t)			\
	a += b; c += d;				\
	b = ROTATE(b, s) ^ a;			\
	d = ROTATE(d, t) ^ c;			\
	a = ROTATE(a, 32);

#define DOUBLE_ROUND(v0,v1,v2,v3)		\
	HALF_ROUND(v0,v1,v2,v3,13,16);		\
	HALF_ROUND(v2,v1,v0,v3,17,21);		\
	HALF_ROUND(v0,v1,v2,v3,13,16);		\
	HALF_ROUND(v2,v1,v0,v3,17,21);


uint64_t siphash24(const void *src, unsigned long src_sz, const char key[16]) {
	const uint64_t *_key = (uint64_t *)key;
	uint64_t k0 = _le64toh(_key[0]);
	uint64_t k1 = _le64toh(_key[1]);
	uint64_t b = (uint64_t)src_sz << 56;
	const uint64_t *in = (uint64_t*)src;

	uint64_t v0 = k0 ^ 0x736f6d6570736575ULL;
	uint64_t v1 = k1 ^ 0x646f72616e646f6dULL;
	uint64_t v2 = k0 ^ 0x6c7967656e657261ULL;
	uint64_t v3 = k1 ^ 0x7465646279746573ULL;

	while (src_sz >= 8) {
		uint64_t mi = _le64toh(*in);
		in += 1; src_sz -= 8;
		v3 ^= mi;
		DOUBLE_ROUND(v0,v1,v2,v3);
		v0 ^= mi;
	}

	uint64_t t = 0; uint8_t *pt = (uint8_t *)&t; uint8_t *m = (uint8_t *)in;
	switch (src_sz) {
	case 7: pt[6] = m[6];
	case 6: pt[5] = m[5];
	case 5: pt[4] = m[4];
	case 4: *((uint32_t*)&pt[0]) = *((uint32_t*)&m[0]); break;
	case 3: pt[2] = m[2];
	case 2: pt[1] = m[1];
	case 1: pt[0] = m[0];
	}
	b |= _le64toh(t);

	v3 ^= b;
	DOUBLE_ROUND(v0,v1,v2,v3);
	v0 ^= b; v2 ^= 0xff;
	DOUBLE_ROUND(v0,v1,v2,v3);
	DOUBLE_ROUND(v0,v1,v2,v3);
	return (v0 ^ v1) ^ (v2 ^ v3);
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
		const uint64_t hash = siphash24(in, size, (const char*)key);
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
		    digest = siphash24(bytes, bc+1, (const char*)key);
	    gettimeofday(&end, NULL);

	    double start_time = (double)start.tv_sec + ((double)start.tv_usec / 1000000);
	    double end_time = (double)end.tv_sec + ((double)end.tv_usec / 1000000);
	    printf("Time for %d bytes: %.6f\n", bc+1, end_time - start_time);
	    }
	}
