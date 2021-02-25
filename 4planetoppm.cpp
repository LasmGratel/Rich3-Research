
// Get-ChildItem *.ppm | foreach { & 'C:\Program Files\ImageMagick-7.0.11-Q16-HDRI\magick.exe' convert $_.Name ($_.Name -replace '\.ppm$','.png')  }
#include <cstdio>
#include <assert.h>
#include <string>

using namespace std;

int uint16toint(uint16_t x)
{
    return x;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s (16.PAT palette file) (4plane file or blk file)\n4 plane file needs to have header", argv[0]);
        return -1;
    }

    const char *palettePath = argv[1];
    const char *blkPath = argv[2];

    FILE *patf, *blkf;
    fopen_s(&patf, palettePath, "rb");
    fopen_s(&blkf, blkPath, "rb");

    char pat[16 * 3];
    fread(pat, sizeof(char), 16 * 3, patf);
    fclose(patf);

    auto ret = fseek(blkf, 0, SEEK_END);
    assert(ret == 0);
    auto length = ftell(blkf);
    assert(length > 0);
    ret = fseek(blkf, 0, SEEK_SET);
    assert(ret == 0);

    printf("Size: %d", length);

    long cursor = 0;
    int i = 0;
    while (length > cursor)
    {
        uint16_t width, height;
        fread(&width, 2, 1, blkf);
        fread(&height, 2, 1, blkf);

        int xsize = uint16toint(width) * 8;
        int ysize = uint16toint(height);

        cursor += 4;
        printf("Width: %d, Height: %d\n", xsize, ysize);

        FILE *imageFile;
        fopen_s(&imageFile, (to_string(i++) + string(".ppm")).c_str(), "wb");

        char *imgdata = (char *)malloc(xsize * ysize / 2);
        fread(imgdata, 1, xsize * ysize / 2, blkf);
        cursor += xsize * ysize / 2;

        fprintf(imageFile, "P6\n%d %d\n255\n", xsize, ysize);
        int plane_size = xsize * ysize / 8;
        int plane_xsize = xsize / 8;

        for (int y = 0; y < ysize; ++y)
        {
            for (int x = 0; x < xsize; ++x)
            {
                int byte = plane_xsize * y + x / 8;
                int index = 7 - (x & 7);
                int bit1 = (imgdata[byte] >> index) & 1;
                int bit2 = (imgdata[byte + plane_size] >> index) & 1;
                int bit3 = (imgdata[byte + plane_size * 2] >> index) & 1;
                int bit4 = (imgdata[byte + plane_size * 3] >> index) & 1;
                int cindex = (bit4 << 3) | (bit3 << 2) | (bit2 << 1) | bit1;
                unsigned char r, g, b;
                r = (unsigned char)pat[cindex * 3] * 4;
                g = (unsigned char)pat[cindex * 3 + 1] * 4;
                b = (unsigned char)pat[cindex * 3 + 2] * 4;
                unsigned char pixels[] = {r, g, b};
                fwrite(pixels, 1, 3, imageFile);
            }
        }
        delete imgdata;
        fclose(imageFile);
    }
    printf("Extracted %d files", i);

    return 0;
}
