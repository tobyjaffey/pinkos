#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WIDTH 4

int main(int argc, char *argv[])
{
    SDL_Surface *image, *imageRGB;
    uint32_t i, y, x;
    uint32_t *pixels;
    FILE *tiles_c, *tiles_h;

    if (argc < 2)
    {
        fprintf(stderr, "Usage %s <filename>\n", argv[0]);
        return 1;
    }

    if (NULL == (image = SDL_LoadBMP(argv[1])))
    {
        fprintf(stderr, "Unable to load bitmap: %s\n", SDL_GetError());
        return 1;
    }

    if (image->w % WIDTH != 0 || image->h != 8 || image->w > 256*WIDTH)
    {
        fprintf(stderr, "Expecting an (n*%d) x 8 image (got %dx%d)\n", WIDTH, image->w, image->h);
        return 1;
    }

    if (NULL == (imageRGB = SDL_CreateRGBSurface(0, image->w, image->h, 32, 0, 0, 0, 0)))
    {
        fprintf(stderr, "Unable to create surface: %s\n", SDL_GetError());
        return 1;
    }

    SDL_BlitSurface(image, NULL, imageRGB, NULL);

    pixels = imageRGB->pixels;

#if 1
    for (y=0;y<imageRGB->h;y++)
    {
        for (x=0;x<imageRGB->w;x++)
        {
            printf("%c", pixels[y*imageRGB->w + x] == 0 ? '0' : '1');
        }
        printf("\n");
    }
#endif

    if (NULL == (tiles_c = fopen("tiles.c", "w")))
    {
        fprintf(stderr, "Can't open tiles.c\n");
        return 1;
    }

    if (NULL == (tiles_h = fopen("tiles.h", "w")))
    {
        fprintf(stderr, "Can't open tiles.h\n");
        return 1;
    }

    fprintf(tiles_h, "#ifndef TILES_H\n");
    fprintf(tiles_h, "#define TILES_H 1\n");
    fprintf(tiles_h, "#include \"common.h\"\n");
    fprintf(tiles_h, "extern const uint8_t tiles[%d][%d];\n", imageRGB->w / WIDTH, WIDTH);
    fprintf(tiles_h, "#endif\n");
    fclose(tiles_h);

    fprintf(tiles_c, "#include \"common.h\"\n");
    fprintf(tiles_c, "#include \"tiles.h\"\n");
    fprintf(tiles_c, "const uint8_t tiles[%d][%d] = \n", imageRGB->w / WIDTH, WIDTH);
    fprintf(tiles_c, "{\n");
    
    for (i=0;i<imageRGB->w;i+=WIDTH)
    {
        fprintf(tiles_c, "\t{");
        for (x=0;x<WIDTH;x++)
        {
            uint8_t line = 0;
            uint8_t mask = 1;
            for (y=0;y<8;y++)
            {
                line |= pixels[ (y * imageRGB->w) + (i + x) ] == 0 ? 0 : mask;
                mask <<= 1;
            }
            fprintf(tiles_c, "0x%02X,", line);
        }
        fprintf(tiles_c, "},\n");
    }

    fprintf(tiles_c, "};\n");
 
    fclose(tiles_c);

    SDL_FreeSurface(imageRGB);
    SDL_FreeSurface(image);
    return 0;
}

