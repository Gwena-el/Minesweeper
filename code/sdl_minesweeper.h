#if !defined(SDL_MINESWEEPER)


#define internal        static
#define local_persist   static
#define global_variable static

#define  u8   uint8_t 
#define  u16  uint16_t
#define  u32  uint32_t
#define  u64  uint64_t

#define  s8   int8_t 
#define  s16  int16_t
#define  s32  int32_t
#define  s64  int64_t


// #########################  PLATFORM LAYER  ####################################
// -----------------------------------------------------------------------------------------
struct Rendering_Context
{
    SDL_Window* window;
    SDL_Surface* back_buffer;
};

#if !USING_SDL_SURFACE
struct BMP_Buffer
{
    void* pixels;
    int w;
    int h;

    int bytes_per_pixels;
    int pitch;
};
#endif

struct info_fps
{
    double ms_per_frame;
    double time_elapsed;
    double fps;
    double mcpf;
};

struct sdl_window_dimension
{
    int width;
    int height;
};

struct rgb_color
{
    float r;
    float g;
    float b;
    float a;
};

enum character
{
    ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
                     
    /* BOMB, */
    /* FLAG, */
    QUESTION_MARK,
    
    MAX_CHARACTERS,
};

#if USING_SDL_SURFACE
struct Assets
{
    SDL_Surface* grid_buffer;
    SDL_Surface* menu_background;

    SDL_Surface* selector;
    SDL_Surface* bomb;
    SDL_Surface* bomb_explode;
    SDL_Surface* flag;
    SDL_Surface* cover;
    
    SDL_Surface* font_asset[MAX_CHARACTERS];    
    SDL_Surface* font_bitmap;
    SDL_Surface* temp_string;

    SDL_Surface* btn_classic;
    SDL_Surface* btn_mine;
    SDL_Surface* btn_parkour;
    SDL_Surface* btn_exit;
    SDL_Surface* btn_return;
    
    
    SDL_Surface* ui_layer;
};
#else
struct Assets
{
    BMP_Buffer grid_buffer;
    BMP_Buffer menu_background;

    BMP_Buffer selector;
    BMP_Buffer bomb;
    BMP_Buffer bomb_explode;
    BMP_Buffer flag;
    BMP_Buffer cover;

    BMP_Buffer font_asset[MAX_CHARACTERS];    
    BMP_Buffer font_bitmap;
    BMP_Buffer temp_string;

    BMP_Buffer btn_classic;
    BMP_Buffer btn_mine;
    BMP_Buffer btn_parkour;
    BMP_Buffer btn_exit;
    BMP_Buffer btn_return;
    
    BMP_Buffer ui_layer;
};
#endif


// -----------------------------------------------------------------------------------------
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

inline SDL_Surface* Create_Surface(int w, int h);

internal SDL_Surface* Load_Bitmap(const char *file, rgb_color *filter);

inline void Bit_Blt(SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect);
internal SDL_Surface* Resize_Surface(SDL_Surface **surface, int w, int h);
internal void SDL_Resize_Window(Rendering_Context *rendering_ctx, int w, int h);

/* internal void Make_String_From_Int(Assets *assets, SDL_Surface *dest_surface, int number_to_stringify, */
/*                                    int x, int y, int display_width, int display_height); */
internal void Get_String_Size(char string[], int scale, int *width, int *height);
internal void Make_String(SDL_Surface *dest_surface, SDL_Surface *font_surface,
                          char text[], int scale, int px, int py, int width, int height);


#define SDL_MINESWEEPER
#endif
