
// -----------------------------------------------------------------------------------------
inline rgb_color rgb_to_float(int r, int g, int b, int a)
{
    rgb_color color = {};
    color.r = (float)r/255;
    color.g = (float)g/255;
    color.b = (float)b/255;
    color.a = (float)a/255;

    return color;
}

// -----------------------------------------------------------------------------------------
internal void Set_Pixel(void *pixels, int pitch, int bytes_per_pixel,
                        int x, int y, rgb_color color)
{
    u32* pixel = (u32 *)pixels + (y * (pitch / bytes_per_pixel) + x);

    *pixel = ((u8)(color.a * 255) << 24) |
             ((u8)(color.r * 255) << 16) |
             ((u8)(color.g * 255) <<  8) |
             ((u8)(color.b * 255) <<  0);
}

// -----------------------------------------------------------------------------------------
internal u32* Get_Pixel(void *pixels, int pitch, int bytes_per_pixel, int x, int y)
{
    u32* pixel = (u32 *)pixels + (y * (pitch / bytes_per_pixel) + x);
    return pixel;
}

// -----------------------------------------------------------------------------------------
internal void Plot_Line_Low(void *pixels, int pitch, int bytes_per_pixel,
                            int x0, int y0, int x1, int y1,
                            rgb_color color)
{
    int dx, dy, yi, D, y, x;
    dx = x1 - x0;
    dy = y1 - y0;
    yi = 1;
    
    if(dy < 0)
    {
        yi = -1;
        dy = -dy;        
    }
    
    D = (2 * dy) - dx;
    x = x0;
    y = y0;

    while(x < x1)
    {
        Set_Pixel(pixels, pitch, bytes_per_pixel, x, y, color);
        if(D > 0)
        {
            y = y + yi;
            D = D + (2 * (dy - dx));
        }
        else
            D = D + 2*dy;

        ++x;
    }
}

// -----------------------------------------------------------------------------------------
internal void Plot_Line_High(void *pixels, int pitch, int bytes_per_pixel,
                             int x0, int y0, int x1, int y1,
                             rgb_color color)
{
    int dx, dy, xi, D, x, y;
    dx = x1 - x0;
    dy = y1 - y0;
    xi = 1;
    if(dx < 0)
    {
        xi = -1;
        dx = -dx;        
    }

    D = (2 * dx) - dy;
    x = x0;
    y = y0;

    while(y < y1)
    {
        Set_Pixel(pixels, pitch, bytes_per_pixel, x, y, color);
        if(D > 0)
        {
            x = x + xi;
            D = D + (2 * (dx - dy));
        }
        else
            D = D + 2*dx;

        ++y;
    }
}

// -----------------------------------------------------------------------------------------
// @IMPORTANT:
// This function is simply to slow! The app get stuck in the while loop and become irresponsive!
// Store the line that must be drawned into a fixed buffer (surface or texture), not to recalculate
// the line every frame.
internal void Draw_Line(void *pixels, int pitch, int bytes_per_pixel,
                        int x0, int y0, int x1, int y1,
                        rgb_color color)
{
    if( abs(y1 - y0) < abs(x1 - x0) )
    {
        if(x0 > x1)
            Plot_Line_Low(pixels, pitch, bytes_per_pixel, x1, y1, x0, y0, color);
        else
            Plot_Line_Low(pixels, pitch, bytes_per_pixel, x0, y0, x1, y1, color);
    }
    else
    {
        if(y0 > y1)
            Plot_Line_High(pixels, pitch, bytes_per_pixel, x1, y1, x0, y0, color);
        else
            Plot_Line_High(pixels, pitch, bytes_per_pixel, x0, y0, x1, y1, color);       
    }   
}

// -----------------------------------------------------------------------------------------
internal void Draw_Grid_Line(void *pixels, int pitch, int bytes_per_pixel,
                             int n_line_x, int n_line_y, int space_x, int space_y,
                             rgb_color color)
{
    int width = n_line_x * space_x;
    int height = n_line_y * space_y;
    
    for(int y = 1; y < n_line_y; ++y) 
        Draw_Line(pixels, pitch, bytes_per_pixel, 0, y * space_y, width, y * space_y, color);
    
    for(int x = 1; x < n_line_x; ++x)
        Draw_Line(pixels, pitch, bytes_per_pixel, x * space_x, 0, x * space_x, height, color);
}


// -----------------------------------------------------------------------------------------
// Does not update the texture! Only draw directly to the pixels in the given buffer.
internal void Draw_Full_Rect(void *pixels, int width, int height, int pitch, rgb_color color)
{
    u8* row = (u8 *)pixels;
    for(int y = 0; y < height; ++y)
    {
        u32* pixel = (u32 *)row;
        for(int x = 0; x < width; ++x)
        {
            *pixel++ = ((u8)(color.a * 255) << 24) |
                       ((u8)(color.r * 255) << 16) |
                       ((u8)(color.g * 255) <<  8) |
                       ((u8)(color.b * 255) <<  0);
        }
        row += pitch;
    }   
}

// -----------------------------------------------------------------------------------------
internal void Draw_Full_Rect_At(void *pixels, int x, int y, int width, int height,
                                int pitch, int bytes_per_pixel,
                                rgb_color color)
{
    if((x + width <= window_width) && (y + height <= window_height))
    {
        u32* pixel_start = Get_Pixel(pixels, pitch, bytes_per_pixel, x, y);
        Draw_Full_Rect(pixel_start, width, height, pitch, color);   
    }
}

// -----------------------------------------------------------------------------------------
internal void Draw_Background_Gradient(void *pixels, int width, int height, int pitch, rgb_color color)
{
    u8* row = (u8 *)pixels;

    int lowest_color_range, red, green, blue;
    red   = (int)(color.r * 255);
    green = (int)(color.g * 255);
    blue  = (int)(color.b * 255);
    
    (red < green) ? lowest_color_range = red : lowest_color_range = green;
    if(blue < lowest_color_range) lowest_color_range = blue;

    float color_decrement = (float)(lowest_color_range - 80) / height;
    
    for(int y = 0; y < height; ++y)
    {
        u32* pixel = (u32 *)row;
        for(int x = 0; x < width; ++x)
        {
            *pixel++ = ((u8)(color.a * 255) << 24) |
                ((u8)(red   - (color_decrement * y)) << 16) |
                ((u8)(green - (color_decrement * y)) <<  8) |
                ((u8)(blue  - (color_decrement * y)) <<  0);
        }
        row += pitch;
    }   
}


// -----------------------------------------------------------------------------------------
// @NOTE: Use this function in case src_buffer as a lower bytes per pixels(bpp) count -> bitmap countaining only the alpha chanel.
//        This assume that dst_buffer has 4 bpp.
internal void Custom_Alpha_Blt(void *src_pixels,
                         int src_x, int src_y, int src_w, int src_h, int src_size_w, int src_size_h,
                         void *dst_pixels,
                         int dst_x, int dst_y, int dst_size_w, int dst_size_h, int dst_pitch)
{
    assert(src_x < src_size_w && src_y < src_size_h);
    assert(dst_x < dst_size_w && dst_y < dst_size_h);

    //assert(src_x + src_w <= src_size_w && src_y + src_h <= src_size_h);
    if(dst_x + src_w > dst_size_w) src_w = dst_size_w - dst_x;
    if(dst_y + src_h > dst_size_h) src_h = dst_size_h - dst_y;

    // @IMPORTANT:  Passing 0 as widht and height: blt the entire data.
    if(src_w == 0 && src_h == 0)
    {
        src_x = 0;
        src_y = 0;
        src_w = src_size_w;
        src_h = src_size_h;
    }

    int src_bpp = 1, src_pitch = src_w;
    int dst_bpp = 4;
    u8* src_start_pixels = (u8*)src_pixels + (src_y * (src_pitch / src_bpp) + src_x);
    u32* dst_start_pixels = Get_Pixel(dst_pixels, dst_x, dst_y, dst_pitch, dst_bpp);
    
    u8* src_row = src_start_pixels;
    u8* dst_row = (u8*)dst_start_pixels;
    // @TODO: Premultiplied alpha?
    for(int y = 0; y < src_h; ++y)  
    {                               
        u8* src_pixel = src_row;             
        u32* dst_pixel = (u32*)dst_row;
        for(int x = 0; x < src_w; ++x)
        {
            float a = (float)*src_pixel / 255.0f;
            u8 sa = *src_pixel;
            // u8 sr = *src_pixel;     //   
            // u8 sg = *src_pixel;     //    white gradiant
            // u8 sb = *src_pixel;     //
            u8 sr = 0x00;     //   
            u8 sg = 0x00;     //    black gradiant
            u8 sb = 0x00;     //

            u8 da = (u8)((*dst_pixel >> 24) & 0xff);
            u8 dr = (u8)((*dst_pixel >> 16) & 0xff);
            u8 dg = (u8)((*dst_pixel >> 8) & 0xff);
            u8 db = (u8)((*dst_pixel >> 0) & 0xff);

            u8 fa = (da < sa) ? sa : da;
            u8 fr = (u8)( (1.0f - a) * (float)dr + (a * (float)sr));
            u8 fg = (u8)( (1.0f - a) * (float)dg + (a * (float)sg));
            u8 fb = (u8)( (1.0f - a) * (float)db + (a * (float)sb));
            
            *dst_pixel = (u32)( (fa << 24) | (fr << 16) | (fg << 8) | (fb << 0) );

            dst_pixel++;
            src_pixel++;
        }
        src_row += src_pitch;
        dst_row += dst_pitch;
    }
}

// -----------------------------------------------------------------------------------------
// @NOTE: No bytes per pixels, this assume rgba format = 4 bytes.
internal void Custom_Blt(void *src_pixels,
                         int src_x, int src_y, int src_w, int src_h, int src_size_w, int src_size_h, int src_pitch,
                         void *dst_pixels,
                         /*int dst_w, int dst_h, We currently don't need this as we don't scale...*/ 
                         int dst_x, int dst_y, int dst_size_w, int dst_size_h, int dst_pitch)
{
    assert(src_x < src_size_w && src_y < src_size_h);
    assert(dst_x < dst_size_w && dst_y < dst_size_h);

    //assert(src_x + src_w <= src_size_w && src_y + src_h <= src_size_h);
    if(dst_x + src_w > dst_size_w) src_w = dst_size_w - dst_x;
    if(dst_y + src_h > dst_size_h) src_h = dst_size_h - dst_y;

    // @IMPORTANT:  Passing 0 as widht and height: blt the entire data.
    if(src_w == 0 && src_h == 0)
    {
        src_x = 0;
        src_y = 0;
        src_w = src_size_w;
        src_h = src_size_h;
    }

    int bytes_per_pixel = 4;
    u32* src_start_pixels = Get_Pixel(src_pixels, src_x, src_y, src_pitch, bytes_per_pixel);
    u32* dst_start_pixels = Get_Pixel(dst_pixels, dst_x, dst_y, dst_pitch, bytes_per_pixel);

    u8* src_row = (u8*)src_start_pixels;
    u8* dst_row = (u8*)dst_start_pixels;

    for(int y = 0; y < src_h; ++y)
    {
        u32* src_pixel = (u32*)src_row;
        u32* dst_pixel = (u32*)dst_row;
        for(int x = 0; x < src_w; ++x)
        {
            float a = (float)(u8)((*src_pixel >> 24) & 0xff) / 255.0f;
            u8 sa = (u8)((*src_pixel >> 24) & 0xff);
            u8 sr = (u8)((*src_pixel >> 16) & 0xff);
            u8 sg = (u8)((*src_pixel >> 8) & 0xff);
            u8 sb = (u8)((*src_pixel >> 0) & 0xff);

            u8 da = (u8)((*dst_pixel >> 24) & 0xff);
            u8 dr = (u8)((*dst_pixel >> 16) & 0xff);
            u8 dg = (u8)((*dst_pixel >> 8) & 0xff);
            u8 db = (u8)((*dst_pixel >> 0) & 0xff);

            u8 fa = (da < sa) ? sa : da;
            u8 fr = (u8)( (1.0f - a) * (float)dr + (a * (float)sr));
            u8 fg = (u8)( (1.0f - a) * (float)dg + (a * (float)sg));
            u8 fb = (u8)( (1.0f - a) * (float)db + (a * (float)sb));
            
            *dst_pixel = (u32)( (fa << 24) | (fr << 16) | (fg << 8) | (fb << 0) );

            dst_pixel++;
            src_pixel++;
        }
        src_row += src_pitch;
        dst_row += dst_pitch;
    }
}

