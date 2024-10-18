#include <SDL2/SDL.h>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#define u32 uint32_t
#define u8  uint8_t  
#define s8  int8_t   

struct buffer_size
{
    u32* buffer;
    int size;
};

static buffer_size Extract_Color_Palette(const char *file)
{
    buffer_size buffer;
    SDL_Surface* bmp_surface = SDL_LoadBMP(file);
    if(bmp_surface)
    {
        u32* current_length = (u32*)bmp_surface->pixels;
        int color_count = 0;
        u8 a = 0, r = 0, g = 0, b = 0;
        while(!(a == 255 && r == 255 && g == 0 && b == 255))
        {
            a = (u8)(*current_length >> 24);
            r = (u8)(*current_length >> 16);
            g = (u8)(*current_length >>  8);
            b = (u8)(*current_length >>  0);

            current_length++;
            color_count++;
        }        
        color_count--;  //Remove the last magenta pixel.
        
        buffer.buffer = (u32 *)malloc(sizeof(u32) * (color_count));
        buffer.size = color_count;
        
        u32* src = (u32*)bmp_surface->pixels;
        u32* dest = buffer.buffer;
        for(int i = 0; i < color_count; ++i)
        {
            a = (u8)(*src >> 24);
            r = (u8)(*src >> 16);
            g = (u8)(*src >>  8);
            b = (u8)(*src >>  0);

            *dest++ = ( (a << 24) | (r << 16) | (g << 8) | (b << 0) );
            src++;
        }
    }
    SDL_FreeSurface(bmp_surface);

    return buffer;
}

int main()
{
    buffer_size buffer = Extract_Color_Palette("color_palette.bmp");
    if(buffer.buffer)
    {
        FILE* file = fopen("../../data/color.dat", "wb");
        if(file)
        {
            fwrite(buffer.buffer, sizeof(u32), buffer.size, file);
            fclose(file);
        }   
    }
}
