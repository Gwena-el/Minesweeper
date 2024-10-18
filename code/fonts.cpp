enum font_ptr
{
    letter = 0,
    w = 1,
    h = 5,
    pixels = 9,
};

struct Font_Data
{
    void* buffer;
    // @NOTE: 93 letters stored, from ! to ~
    // space MUST be dealt with separatly.
    int offset[93];
};

internal void extract_font_data(Font_Data *font_data, char *path_to_font)
{
    FILE* font_file = fopen(path_to_font, "rb");
    if(font_file)
    {
        fseek(font_file, 0, SEEK_END);
        size_t filesize = ftell(font_file);
        fseek(font_file, 0, SEEK_SET);

        font_data->buffer = (u8*)malloc(filesize);
        if(font_data->buffer)
        {
            fread(font_data->buffer, filesize, 1, font_file);
            fclose(font_file);
            
            // 93 letter stored
            char ascii_table[] = "!\"#$%&'()*+,-./"
                                 "0123456789:;<=>?"
                                 "@ABCDEFGHIJKLMNO"
                                 "PQRSTUVWXYZ[\\]^_"
                                 "`abcdefghijklmno"
                                 "pqrstuvwxyz{|}~";
            int c = 0, i = 0;
            u8* current_ptr = (u8*)font_data->buffer;
            while(c < 93)
            {
                assert(ascii_table[c] == (char)*current_ptr);
                
                font_data->offset[c++] = i;
                current_ptr++; // ignore letter
                int w = (int)(current_ptr[0] | current_ptr[1] | current_ptr[2] | current_ptr[3]);
                current_ptr += 4;
                int h = (int)(current_ptr[0] | current_ptr[1] | current_ptr[2] | current_ptr[3]);
                current_ptr += 4;
                i += 9;

                int char_pixels_size = w * h;
                i += char_pixels_size;
                current_ptr += char_pixels_size;
            }
        }
        else fprintf(stderr, "Could'nt allocate buffer.\n");
    }
    else fprintf(stderr, "Could'nt load font data file in memory.\n");
}

// -----------------------------------------------------------------------------------------
internal void* get_char_bitmap(Font_Data *font_data, char c, int *w, int *h)
{
    int ascii_index = c - 33;
    if(ascii_index < 0) return NULL;
    
    u8* ptr_to_char_info = (u8*)font_data->buffer + font_data->offset[ascii_index];
    *w = (int)(ptr_to_char_info[font_ptr::w] | ptr_to_char_info[font_ptr::w + 1] | ptr_to_char_info[font_ptr::w + 2] | ptr_to_char_info[font_ptr::w + 3]);
    *h = (int)(ptr_to_char_info[font_ptr::h] | ptr_to_char_info[font_ptr::h + 1] | ptr_to_char_info[font_ptr::h + 2] | ptr_to_char_info[font_ptr::h + 3]);
    return (ptr_to_char_info + font_ptr::pixels);
}

// -----------------------------------------------------------------------------------------
internal void get_string_size(Font_Data *font_data, char *string, int *w, int *h)
{
    int c = 0, max_w = 0, max_h = 0, temp_w, temp_h;
    
    while(string[c]) // hoping for null termination
    {
        if(string[c] == ' ') max_w += 10;            // space
        else
        {
            if(get_char_bitmap(font_data, string[c], &temp_w, &temp_h))
            {             
                max_w += temp_w;
                if(temp_h > max_h) max_h = temp_h;      
            }
            else fprintf(stderr, "String size error: unknown character.");
        }
        c++;
    }

    *w = max_w;
    *h = max_h;    
}

// -----------------------------------------------------------------------------------------
// @NOTE: The anti-ailiasing causes certain letter(curved) to appeares offset even thought they aren't
// because the last pixel is barrely visible.
// It's fine for printing single letter, but looks terrible when printing string.
internal void* get_string_bitmap(Font_Data *font_data, char *string, int *w, int *h)
{
    get_string_size(font_data, string, w, h);

    u32* bitmap = (u32*) malloc(sizeof(u32) * (*w) * (*h));
    if(bitmap)
    {
        memset(bitmap, 0, sizeof(u32) * (*w) * (*h));
        
        int current_w = 0, temp_w, temp_h;
        int c = 0;
        while(string[c])
        {
            if(string[c] == ' ') current_w += 10;      // space
            else
            {
                u8* temp_bitmap = (u8*)get_char_bitmap(font_data, string[c], &temp_w, &temp_h);
                if(temp_bitmap)
                {
                    int actual_h = *h - temp_h;
                    Custom_Alpha_Blt(temp_bitmap, 0,0,0,0, temp_w, temp_h,
                                     bitmap, current_w, actual_h, *w, *h, *w * 4);
                    current_w += temp_w;   
                }
                else fprintf(stderr, "Character bitmap error: unknown character.");
            }
            c++;
        }

        return bitmap;
    }
    else fprintf(stderr, "Couldn't allocate string bitmap memory.");
    return NULL;
}
