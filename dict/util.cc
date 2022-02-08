#include "util.h"
#include "digest.h"

bool hmac_key_set = false;
uint8 shared_hmac_md5_key[16];

bool siphash_key_set = false;
uint8 shared_siphash_key[SIPHASH_KEYLEN];

void hmac_md5(size_t size, const unsigned char* bytes, unsigned char digest[16])
	{
	if ( ! hmac_key_set )
		printf("HMAC-MD5 invoked before the HMAC key is set");

	internal_md5(bytes, size, digest);

	for ( int i = 0; i < 16; ++i )
		digest[i] ^= shared_hmac_md5_key[i];

	internal_md5(digest, 16, digest);
	}
