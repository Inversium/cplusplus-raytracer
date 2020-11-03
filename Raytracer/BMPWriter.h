#pragma once

#include "math/Headers/mvector.h"
#include <fstream>

#define PIXELDEPTH 24
#define include_padding(h, w) h * (w + ((4 - (w * PIXELDEPTH / 8) % 4) % 4)) * PIXELDEPTH

struct bmphdr
{
    uint16_t    bfType;
    uint32_t    bfSize;
    uint32_t    bfReserved_1;
    uint32_t    bfReserved_2;
    uint32_t    bfOffBits;
    uint32_t    biSize;
    uint32_t    biWidth;
    int32_t     biHeight;
    uint16_t    biPlanes;
    uint16_t    biBitCount;
    uint32_t    biCompression;
    uint32_t    biSizeImage;
    int32_t     biXPelsPerMeter;
    int32_t     biYPelsPerMeter;
    uint32_t    biClrUsed;
    uint32_t    biClrImportant;
};

const unsigned char* make_header(const unsigned int height, const unsigned int width)
{
    bmphdr header; 

    header.bfType = 0x424D; //BM
    header.bfSize = sizeof(header) + include_padding(height, width);
    header.bfReserved_1 = 0;
    header.bfReserved_2 = 0;
    header.bfOffBits = sizeof(header);
    header.biSize = 40;
    header.biWidth = width;
    header.biHeight = -(int)height; //negative because otherwise image will be horizontally flipped
    header.biPlanes = 1;
    header.biBitCount = PIXELDEPTH;
    header.biCompression = 0;
    header.biSizeImage = include_padding(height, width);
    header.biXPelsPerMeter = 2835;
    header.biYPelsPerMeter = 2835;
    header.biClrUsed = 0;
    header.biClrImportant = 0;
    

    unsigned char* output = new unsigned char[54];

    output[0] = header.bfType >> 8;
    output[1] = header.bfType & 0xFF;

    output[2] = header.bfSize;
    output[3] = header.bfSize >> 8;
    output[4] = header.bfSize >> 16;
    output[5] = header.bfSize >> 24;

    output[6] = header.bfReserved_1;
    output[7] = header.bfReserved_1 >> 8;
    output[8] = header.bfReserved_2;
    output[9] = header.bfReserved_2 >> 8;

    output[10] = header.bfOffBits;
    output[11] = header.bfOffBits >> 8;
    output[12] = header.bfOffBits >> 16;
    output[13] = header.bfOffBits >> 24;


    output[14] = header.biSize;
    output[15] = header.biSize >> 8;
    output[16] = header.biSize >> 16;
    output[17] = header.biSize >> 24;

    output[18] = header.biWidth;
    output[19] = header.biWidth >> 8;
    output[20] = header.biWidth >> 16;
    output[21] = header.biWidth >> 24;

    output[22] = header.biHeight;
    output[23] = header.biHeight >> 8;
    output[24] = header.biHeight >> 16;
    output[25] = header.biHeight >> 24;

    output[26] = header.biPlanes;
    output[27] = header.biPlanes >> 8;

    output[28] = header.biBitCount;
    output[29] = header.biBitCount >> 8;

    output[30] = header.biCompression;
    output[31] = header.biCompression >> 8;
    output[32] = header.biCompression >> 16;
    output[33] = header.biCompression >> 24;

    output[34] = header.biSizeImage;
    output[35] = header.biSizeImage >> 8;
    output[36] = header.biSizeImage >> 16;
    output[37] = header.biSizeImage >> 24;

    output[38] = header.biXPelsPerMeter;
    output[39] = header.biXPelsPerMeter >> 8;
    output[40] = header.biXPelsPerMeter >> 16;
    output[41] = header.biXPelsPerMeter >> 24;

    output[42] = header.biYPelsPerMeter;
    output[43] = header.biYPelsPerMeter >> 8;
    output[44] = header.biYPelsPerMeter >> 16;
    output[45] = header.biYPelsPerMeter >> 24;

    output[46] = header.biClrUsed;
    output[47] = header.biClrUsed >> 8;
    output[48] = header.biClrUsed >> 16;
    output[49] = header.biClrUsed >> 24;

    output[50] = header.biClrUsed;
    output[51] = header.biClrImportant >> 8;
    output[52] = header.biClrImportant >> 16;
    output[53] = header.biClrImportant >> 24;

    return output;
}

void CreateImage(vec3* framebuffer, const unsigned int height, const unsigned int width, const char* FileName = "output.bmp")
{
    FILE* bmp = nullptr;
    fopen_s(&bmp, FileName, "wb");

    unsigned char padding = 0;
    int PaddingSize = (4 - (width * (PIXELDEPTH / 8)) % 4) % 4;

    fwrite(make_header(height, width), sizeof(bmphdr), 1, bmp);

    //std::cout << make_header(height, width);
	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
            unsigned char R = static_cast<unsigned char>(framebuffer[i * width + j].x * 255);
            unsigned char G = static_cast<unsigned char>(framebuffer[i * width + j].y * 255);
            unsigned char B = static_cast<unsigned char>(framebuffer[i * width + j].z * 255);

            fwrite(&B, 1, 1, bmp);
            fwrite(&G, 1, 1, bmp);
            fwrite(&R, 1, 1, bmp);           
		}
        fwrite(&padding, 1, PaddingSize, bmp);
	}

    fclose(bmp);
}