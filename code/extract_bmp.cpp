struct BMP_File_Header
{
    // BMP Header
    u16 id;
    u32 bmp_file_size;  // header + data
    u16 reserved1;
    u16 reserved2;
    u32 offset_to_pixel_data;

    // DIB Header
    u32 bytes_in_dib;
    s32 width;  // left to right
    s32 height; // bottom to top
    u16 nth_color_planes;
    u16 bits_per_pixel;

#if 0    
    u32 compression;
    u32 image_size;
    s32 horizontal_pixels_per_meter;
    s32 vertical_pixels_per_meter;
    u32 nth_colors_in_palette;
    u32 important_color;

    // @TODO: Do I need this?
    // check endianess
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
    u32 alpha_mask;
    u32 color_space_type;
#endif

    // Gamma... 12 bytes
} __attribute__ ((__packed__));

// struct BMP_Buffer
// {
//     void* pixels;
//     int w;
//     int h;

//     int bytes_per_pixels;
//     int pitch;
// };

// struct rgb_color
// {
//     float r;
//     float g;
//     float b;
//     float a;
// };

internal bool Load_BMP(const char *path, BMP_Buffer *bmp_buffer, rgb_color *filter)
{
    bool success = false;

    BMP_File_Header* bmp_header;
    FILE* file = fopen(path, "r");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        int filesize = ftell(file);
        fseek(file, 0, SEEK_SET);

        u8* buffer = (u8 *)malloc(filesize);
        fread(buffer, 1, filesize, file);
        fclose(file);

        bmp_header = (BMP_File_Header *)buffer;
        
        bmp_buffer->w = bmp_header->width;
        bmp_buffer->h = bmp_header->height;
        bmp_buffer->bytes_per_pixels = bmp_header->bits_per_pixel/8;
        bmp_buffer->pitch = bmp_buffer->w * bmp_buffer->bytes_per_pixels;
        bmp_buffer->pixels = (u32 *)malloc(bmp_buffer->w * bmp_buffer->h * bmp_buffer->bytes_per_pixels);
       
        u32* reversed_buffer = (u32 *)((u8 *)buffer + bmp_header->offset_to_pixel_data);        
        u32* src_row = reversed_buffer + (bmp_buffer->w *(bmp_buffer->h - 1));
        u8* dest_row = (u8*)bmp_buffer->pixels;
        for(int y = 0; y < bmp_buffer->h; ++y)
        {
            u32* dest = (u32*)dest_row;
            u32* current_pixel = src_row;

            for(int x = 0; x < bmp_buffer->w; ++x)
            {
                if(*current_pixel != 0 && filter)
                {
                    u8 a = (u8)((*current_pixel >> 24) * filter->a);
                    u8 r = (u8)((*current_pixel >> 16) * filter->r);
                    u8 g = (u8)((*current_pixel >>  8) * filter->g);
                    u8 b = (u8)((*current_pixel >>  0) * filter->b);
                    *current_pixel = ( (a << 24) | (r << 16) | (g << 8) | (b << 0) );
                }
                *dest++ = *current_pixel++;
            }                
                
            
            dest_row += bmp_buffer->pitch;
            src_row -= bmp_buffer->w;
        }

        free(buffer);
        success = true;
    }

    return success;
}
