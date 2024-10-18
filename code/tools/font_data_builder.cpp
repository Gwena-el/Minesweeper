
#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"
#include <cstdio>
#include <cstdint>

// @TODO: Space character isn't represented (w=0, h=0), must deal with it separatly.
char ascii_table[] = "!\"#$%&'()*+,-./"
                     "0123456789:;<=>?"
                     "@ABCDEFGHIJKLMNO"
                     "PQRSTUVWXYZ[\\]^_"
                     "`abcdefghijklmno"
                     "pqrstuvwxyz{|}~";


struct char_info
{
    char letter;
    int w;
    int h;
    uint8_t* bitmap;

    char_info* next;
};

int arg_scale;

// -----------------------------------------------------------------------------------------
static void make_font_data_file(stbtt_fontinfo *font, char_info **head, float scale, char *output_path)
{
    int i = 0;
    while(ascii_table[i])
    {
        int xoff, yoff;
        
        char_info* new_char = (char_info*)malloc(sizeof(char_info));
        new_char->letter = ascii_table[i];
        new_char->next = NULL;
        new_char->bitmap = stbtt_GetCodepointBitmap(font, 0, scale, ascii_table[i++],
                                                    &new_char->w, &new_char->h,
                                                    &xoff, &yoff);
       
        if(!*head) *head = new_char;
        else
        {
            char_info* current_char = *head;
            while(current_char->next)
                current_char = current_char->next;

            current_char->next = new_char;
        }
    }

    int buffer_size = 0;    
    char_info* current_char = *head;
    while(current_char)
    {
        // header infos
        buffer_size += sizeof(char) + (sizeof(int) * 2);
        // bitmap size
        buffer_size += (current_char->w * current_char->h);

        current_char = current_char->next;
    }

    uint8_t* buffer = (uint8_t*)malloc(buffer_size);
    if(buffer)
    {
        uint8_t* ptr = buffer;
        current_char = *head;

        while(current_char)
        {
            *ptr++ = (uint8_t)current_char->letter;

            
            // Ugly ugly
            *ptr++ = (uint8_t)(current_char->w >> 24);
            *ptr++ = (uint8_t)(current_char->w >> 16);
            *ptr++ = (uint8_t)(current_char->w >> 8);
            *ptr++ = (uint8_t)(current_char->w >> 0);
            *ptr++ = (uint8_t)(current_char->h >> 24);
            *ptr++ = (uint8_t)(current_char->h >> 16);
            *ptr++ = (uint8_t)(current_char->h >> 8);
            *ptr++ = (uint8_t)(current_char->h >> 0);

            uint8_t* src_bitmap = (uint8_t*)current_char->bitmap;
            for(int i = 0; i < current_char->w * current_char->h; ++i)
            {
                *ptr++ = *src_bitmap++;   
            }
            //ptr += (current_char->w * current_char->h);

            current_char = current_char->next;
        }

        char font_data_save_path[100];
        sprintf(font_data_save_path, "%s/font_data_%d.dat", output_path, arg_scale);
        FILE* font_data = fopen(font_data_save_path, "wb");
        if(font_data)
        {
            fwrite(buffer, 1, buffer_size, font_data);
            fclose(font_data);
        }
        else fprintf(stderr, "Couldn't create or open file.\n");
    }
    else fprintf(stderr, "Couldn't allocate data buffer.\n");
    

#if 1
    current_char = *head;
    while(current_char)
    {
        printf("letter: %c, w=%d, h=%d\n", current_char->letter, current_char->w, current_char->h);
        current_char = current_char->next;
    }
#endif
}


// -----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{    
    char* default_font_path = (char *)"/home/Gwena/.local/share/fonts/ttf/JetBrainsMono-Regular.ttf";
    char* output_path;
    float scale;

    if(argc == 3)
    {
        sscanf(argv[1], "%f", &scale);
        output_path = argv[2];
    }
    else if(argc == 4)
    {
        default_font_path = argv[1];
        sscanf(argv[2], "%f", &scale);
        output_path = argv[3];
    }
    else
    {
        fprintf(stderr, "Must provide path to font, a scale, and an output path,"
                "path to font can be ignored(default used).\n");
        return 0;
    } 
    
    arg_scale = (int)scale; // for saving file.

    stbtt_fontinfo font;
    FILE* file = fopen(default_font_path, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        size_t filesize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* ttf_buffer = (char*)malloc(filesize);
        if(ttf_buffer)
        {
            fread(ttf_buffer, filesize, 1, file);
            fclose(file);

            int init_font = stbtt_InitFont(&font, (uint8_t *)ttf_buffer,
                                           stbtt_GetFontOffsetForIndex((uint8_t *)ttf_buffer, 0));

            scale = stbtt_ScaleForPixelHeight(&font, scale);

            char_info* head = NULL;
            make_font_data_file(&font, &head, scale, output_path);
            fprintf(stdout, "Should have worked...\n");
        }
    }
    else fprintf(stderr, "Could'nt allocate font buffer.\n");
    
    return 0;
}
