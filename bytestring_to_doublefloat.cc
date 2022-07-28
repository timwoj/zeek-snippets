#include <cassert>
#include <cstdio>
#include <cstring>
#include <limits>
#include <iostream>
#include <vector>

inline float ntohf(float f)
	{
	assert(sizeof(f) == 4);

	float tmp;
	char* src = (char*)&f;
	char* dst = (char*)&tmp;

	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];

	return tmp;
	}

inline double ntohd(double d)
	{
	assert(sizeof(d) == 8);

	double tmp;
	char* src = (char*)&d;
	char* dst = (char*)&tmp;

	dst[0] = src[7];
	dst[1] = src[6];
	dst[2] = src[5];
	dst[3] = src[4];
	dst[4] = src[3];
	dst[5] = src[2];
	dst[6] = src[1];
	dst[7] = src[0];

	return tmp;
	}

int main() {

    char str[4];
    std::vector<float> fs = {3.14e15, -3.14e15, 4e-38, 0.0, -0.0,
                             std::numeric_limits<float>::infinity(),
                             std::numeric_limits<float>::infinity()*-1,
                             std::numeric_limits<float>::quiet_NaN(),
                             std::numeric_limits<float>::denorm_min()};
    for ( float ff : fs )
    {
        float f = ntohf(ff);
        memcpy(str, &f, sizeof(float));

        for (int i = 0; i < 4; i++)
            printf("%02x ", static_cast<uint8_t>(str[i]) & 0xFF);
        printf("\n");
    }

    printf("\n\n");

    char str2[8];
    std::vector<double> ds = {3.14e15, -3.14e15, 4e-308, 0.0, -0.0,
                             std::numeric_limits<double>::infinity(),
                             std::numeric_limits<double>::infinity()*-1,
                             std::numeric_limits<double>::quiet_NaN(),
                             std::numeric_limits<double>::denorm_min()};
    for ( double dd : ds )
    {
        double d = ntohd(dd);
        memcpy(str2, &d, sizeof(double));

        for (int i = 0; i < 8; i++)
            printf("%02x ", static_cast<uint8_t>(str2[i]) & 0xFF);
        printf("\n");
    }
}
