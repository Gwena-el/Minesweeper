//###########################################################################################
/*
@IMPORTANT: Font Rasterizer
            In Command line pass as argument :
            
            1 - Path to the truetype font ( 0 if you want default ).
            2 - Height size in pixel used by stb_truetype to define scale.
            3 - All the letters you need.

@NOTE:      The font are saved as bmp, name has "letter"_.bmp in dir ..data/Font.
*/
//###########################################################################################

#include <cstdio>
#include <cstdint>
#include <SDL2/SDL.h>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define rmask 0xff000000
    #define gmask 0x00ff0000
    #define bmask 0x0000ff00
    #define amask 0x000000ff
#else
    #define rmask 0x000000ff
    #define gmask 0x0000ff00
    #define bmask 0x00ff0000
    #define amask 0xff000000
#endif

// -----------------------------------------------------------------------------------------
static void Make_Font_BMP(stbtt_fontinfo *font, char arg, float scale)
{
    int width, height, xoff, yoff;
    uint8_t* bitmap = stbtt_GetCodepointBitmap(font, 0, scale, arg,
                                               &width, &height,
                                               &xoff, &yoff);

    SDL_Surface* temp_surface = SDL_CreateRGBSurface(0, width, height, 32,
                                                     rmask, gmask, bmask, amask);

    uint8_t* source = bitmap;
    uint8_t* row = (uint8_t *)temp_surface->pixels;
    for(int y = 0; y < height; ++y)
    {
        uint32_t* pixel = (uint32_t *)row;
        for(int x = 0; x < width; ++x)
        {                        
            uint8_t alpha = *source++;
            *pixel++ = ( (alpha << 24) |
                         (alpha << 16) |
                         (alpha << 8)  |
                         (alpha << 0) );
        }
        row += temp_surface->pitch;
    }

    stbtt_FreeBitmap(bitmap, 0);

    char filename[20] = "../data/Font/0_.bmp";
    filename[13] = arg;
    FILE* new_file = fopen(filename, "wb");

    int save_success =  SDL_SaveBMP(temp_surface, filename);
    if(save_success != 0)
    {
        printf("Error: Could not save file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fclose(new_file);
    SDL_FreeSurface(temp_surface);      
}

// -----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        stbtt_fontinfo font;
        FILE* file;
        char* default_font_path = (char *)"/home/Gwena/.local/share/fonts/ttf/JetBrainsMono-Regular.ttf";
        
        if(argv[1] == 0)
            file = fopen(default_font_path, "rb");
        else
            file = fopen(argv[1], "rb");
        
        if(!file)
            file = fopen(default_font_path, "rb");
        
        size_t file_size;
        if(file)
        {
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            char *ttf_buffer = (char *)malloc(file_size);
            if(ttf_buffer)
            {
                fread(ttf_buffer, file_size, 1, file);
                fclose(file);

                int init_font_success = stbtt_InitFont(&font, (uint8_t *)ttf_buffer,
                                                       stbtt_GetFontOffsetForIndex((uint8_t *)ttf_buffer, 0));

                float scale_size;
                sscanf(argv[2], "%f", &scale_size);
                float scale = stbtt_ScaleForPixelHeight(&font, scale_size);

                       
                for(int i = 3; i < argc; ++i)
                {
                    Make_Font_BMP(&font, *argv[i], scale);            
                }

                free(ttf_buffer);
            }
            else
            {
                printf("Could not allocate font buffer!");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            printf("Could not load font!");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("Error: No arguments provided.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
