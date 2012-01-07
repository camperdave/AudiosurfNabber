/*
Copyright (c) 2008 Philip Taylor (excors@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <zlib.h>

#ifdef _MSC_VER
typedef __int32 int32_t;
#endif

int usage()
{
    printf("Usage error: Should be run like 'convert-cgr.exe something.cgr'\n");
    exit(1);
}

void die(const char* msg)
{
    printf("%s\n", msg);
    exit(1);
}

int main(int argc, char** argv)
{
    printf("Audiosurf CGR Ogg extractor - originally by Philip Taylor (excors@gmail.com)\n\n");

    if (argc != 2)
        usage();

    const char* filename_in = argv[1];
    if (strcmp(filename_in + strlen(filename_in) - 4, ".cgr"))
        usage();

    char* filename_out = strdup(filename_in);
    strcpy(filename_out + strlen(filename_out) - 4, ".ogg");

    FILE* file_in = fopen(filename_in, "rb");
    if (! file_in) {
        printf("Error: Cannot open file '%s'\n", filename_in);
        exit(1);
    }

    FILE* file_out = fopen(filename_out, "wb");
    if (! file_out) {
        printf("Error: Cannot open file '%s'\n", filename_out);
        exit(1);
    }

    int32_t output_size = -1;
    printf("Looking for ZIOS and ZICB chunks...\n");

    char chunk[5] = { 0 };
    int32_t chunk_length;
    while (1) {
        if (fread(chunk, 4, 1, file_in) != 1) die("Input error");
        if (fread(&chunk_length, 4, 1, file_in) != 1) die("Input error");
        printf("Found %s\n", chunk);

        if (memcmp(chunk, "ZIOS", 4) == 0) {
            if (chunk_length != 4) die("ZIOS chunk has wrong size");
            if (fread(&output_size, 4, 1, file_in) != 1) die("Input error");

        } else if (memcmp(chunk, "ZICB", 4) == 0) {
            if (output_size < 0) die("ZIOS chunk not found before ZICB");

            printf("Decompressing data...\n");

            Bytef* input = (Bytef*)malloc(chunk_length);
            if (! input) die("Error: Failed to allocate memory for input");
            if (fread(input, chunk_length, 1, file_in) != 1) die("Input error");

            Bytef* output = (Bytef*)malloc(output_size);
            if (! output) die("Error: Failed to allocate memory for output");

            uLongf actual_output_length;
            if (uncompress(output, &actual_output_length, input, chunk_length) != Z_OK)
                die("Decompression error");
            if (actual_output_length != output_size)
                die("Error: Decompressed output has incorrect size");

            const char* p = (const char*)output + 16;
            printf("Looking for BUFV chunk...\n");
            while (p < (const char*)output + output_size) {
                memcpy(chunk, p, 4);
                p += 4;
                memcpy(&chunk_length, p, 4);
                p += 4;
                printf("Found %s\n", chunk);
                if (memcmp(chunk, "BUFV", 4) == 0) {
                    printf("Writing output\n");
                    if (fwrite(p, chunk_length, 1, file_out) != 1) die("Output error");
                    fclose(file_out);
                    printf("Finished\n");
                    exit(0);
                }
                p += chunk_length;
            }
            die("Error: Failed to find BUFV");

        } else {
            if (fseek(file_in, chunk_length, SEEK_CUR)) die("Input error");
        }
    }
}
