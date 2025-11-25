#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <cctype>

#include <openssl/evp.h>
#include <openssl/sha.h>

int
b64_pton(const char *src, u_char *dst, size_t dstsiz)
{
	int		 i, j, k;
	uint32_t	 val3 = 0;
	const char	 b64_tbl[] =
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x3e\xff\xff\xff\x3f"
	    "\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\xff\xff\xff\x00\xff\xff"
	    "\xff\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e"
	    "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\xff\xff\xff\xff\xff"
	    "\xff\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28"
	    "\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

	for (i = j = k = 0; src[i] != '\0'; i++) {
		if (isspace((unsigned char)src[i]))
			continue;
		if (b64_tbl[(u_char)src[i]] == (char)0xff)
			return(-1);
		val3 |= b64_tbl[(u_char)src[i]];
		if (src[i] != '=') {
			if (dst != NULL && k >= (ssize_t)dstsiz)
				return (-1);
			if (j % 4 == 1) {
				if (dst != NULL)
					dst[k] = val3 >> 4;
				k++;
			} else if (j % 4 == 2) {
				if (dst != NULL)
					dst[k] = val3 >> 2;
				k++;
			} else if (j % 4 == 3) {
				if (dst != NULL)
					dst[k] = val3;
				k++;
			}
		}
		val3 <<= 6;
		j++;
	}
	if (j % 4 != 0)
		return (-1);

	return (k);
}

int
b64_ntop(u_char *src, size_t srclength, char *target, size_t target_size)
{
	int		 i, j;
	size_t		 expect_siz;
	uint32_t	 bit24;
	const char	 b64str[] =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	expect_siz = ((srclength + 2) / 3) * 4 + 1;

	if (target == NULL)
		return ((int)expect_siz);
	if (target_size < expect_siz)
		return (-1);

	for (i = 0, j = 0; i < srclength; i += 3) {
		bit24 = src[i] << 16;
		if (i + 1 < srclength)
			bit24 |= src[i + 1] << 8;
		if (i + 2 < srclength)
			bit24 |= src[i + 2];

		target[j++] = b64str[(bit24 & 0xfc0000) >> 18];
		target[j++] = b64str[(bit24 & 0x03f000) >> 12];
		if (i + 1 < srclength)
			target[j++] = b64str[(bit24 & 0x000fc0) >> 6];
		else
			target[j++] = '=';
		if (i + 2 < srclength)
			target[j++] = b64str[(bit24 & 0x00003f)];
		else
			target[j++] = '=';
	}
	target[j] = '\0';

	return j;
}

void hash_sha256(uint8_t* input, int input_len, uint8_t* output) {
    EVP_MD_CTX* c = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();

    EVP_DigestInit_ex(c, md, nullptr);
    EVP_DigestUpdate(c, input, input_len);
    EVP_DigestFinal(c, output, nullptr);
    EVP_MD_CTX_free(c);
}

int main(int argc, char** argv) {
    FILE* f = fopen(argv[1], "r");
    if ( ! f ) {
        printf("Failed to open file\n");
        return 1;
    }

    char input[128];
    size_t bytes_read = fread(input, 128, 1, f);

    char* data_ptr = strchr(input, 0x20) + 1;
    char* data_end = strchr(data_ptr, 0x20);
    size_t data_len = data_end-data_ptr;

    char data[128];
    strncpy(data, data_ptr, data_len);
    data[data_len] = '\0';

    uint8_t decoded[128];
    size_t decoded_length = b64_pton(data, decoded, 128);

    if ( decoded_length == -1 ) {
        printf("b64_pton failed\n");
        return 1;
    }

    uint8_t sha[SHA256_DIGEST_LENGTH];
    hash_sha256(decoded, decoded_length, sha);

    char encoded[128];
    b64_ntop(sha, SHA256_DIGEST_LENGTH, encoded, 128);

    printf("output: %s\n", encoded);
}
