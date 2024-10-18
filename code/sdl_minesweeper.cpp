#include <SDL2/SDL.h>
#include <cstdio>
#include <stdint.h>
#include <x86intrin.h>
#include <assert.h>
#include <time.h>


#define USING_SDL_SURFACE 1

#if USING_SDL_SURFACE
// Access SDL_Surface BytesPerPixel as I would do with a custom buffer.
#define bytes_per_pixels format->BytesPerPixel
#endif


#include "sdl_minesweeper.h"

global_variable int window_width  = 1280;    // 36 * 32
global_variable int window_height = 720;     // 24 * 32


#include "draw.cpp"
#include "dynamic_array.h"

#if !USING_SDL_SURFACE
#include "extract_bmp.cpp"
#endif

#include "fonts.cpp"
#include "minesweeper.cpp"

// -----------------------------------------------------------------------------------------
//global_variable SDL_Surface* font_bmp;


// -----------------------------------------------------------------------------------------
static void Make_Grid_BMP(int nw, int nh, int cw, int ch)
{
    SDL_Surface* temp_surface = Create_Surface(nw * cw, nh * ch);
    
    for(int y = 0; y < nh; ++y)
    {
        for(int x = 0; x < nw; ++x)
        {
            int i = y * nw + x;            
            int px = x * cw;
            int py = y * ch;

            rgb_color grid_color = {};
            bool grid_maker = (nw & 1) ? (i & 1) : ((i+y) & 1); // make grid whether with is pair or impair.
            if(grid_maker)
            {
                grid_color = game_colors[colors::BACKGROUND_GRID];
            }
            else
            {
                int a = 255, r = 0, g = 0, b = 0;
                assert((game_colors[colors::BACKGROUND_GRID].r * 255 + 16) <= 255 &&
                       (game_colors[colors::BACKGROUND_GRID].g * 255 + 16) <= 255 &&
                       (game_colors[colors::BACKGROUND_GRID].b * 255 + 16) <= 255);
                r = (game_colors[colors::BACKGROUND_GRID].r * 255) + 16;
                g = (game_colors[colors::BACKGROUND_GRID].g * 255) + 16;
                b = (game_colors[colors::BACKGROUND_GRID].b * 255) + 16;

                grid_color = rgb_to_float(r, g, b, a);
            }
            Draw_Full_Rect_At(temp_surface->pixels,
                              px, py, cw, ch,
                              temp_surface->pitch, temp_surface->bytes_per_pixels,
                              grid_color);
        }
    }

    SDL_SaveBMP(temp_surface, "grid_board.bmp");
    SDL_free(temp_surface);
}

#if USING_SDL_SURFACE
// -----------------------------------------------------------------------------------------
inline void Bit_Blt(SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dst, SDL_Rect *dst_rect)
{
    SDL_BlitScaled(src, src_rect, dst, dst_rect);
}
#else
inline void Bit_Blt(BMP_Buffer *src, SDL_Rect *src_rect, BMP_Buffer *dst, BMP_Buffer *dst_rect)
{
    int src_x, src_y, src_w, src_h, dst_x, dst_y;
    if(!src_rect)
    {
        src_x = 0;
        src_y = 0;
        src_w = 0;
        src_h = 0;
    }
    if(!dst_rect)
    {
        dst_x = 0;
        dst_y = 0;
    }
    Custom_Blt(src->pixels,
               src_x, src_y, src_w, src_h, src->w, src->h, src->pitch,
               dst->pixels,
               dst_x, dst_y, dst->w, dst->h, dst->pitch);
}
#endif
     
#if USING_SDL_SURFACE
// -----------------------------------------------------------------------------------------
inline SDL_Surface* Create_Surface(int w, int h)
{
    SDL_Surface* output;
    SDL_Surface* temp_surface = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    output = SDL_ConvertSurface(temp_surface, format, 0);
    SDL_FreeFormat(format);
    SDL_FreeSurface(temp_surface);
    
    return output;    
}
#else
// -----------------------------------------------------------------------------------------
inline void Create_BMP_Buffer(BMP_Buffer *buffer, int w, int h)
{
    buffer->pixels = (u32*) malloc(sizeof(u32) * w * h);
    memset(buffer->pixels, 0, sizeof(u32) * w * h);
    buffer->w = w;
    buffer->h = h;
    buffer->pitch = w * 4; // 4 bytes_per_pixels ARGB
    buffer->bytes_per_pixels = 4;
}
#endif

// -----------------------------------------------------------------------------------------
internal void Center_Window_On_N_Displays(int window_width, int window_height, int &x, int &y)
{
    // enumerate displays
    int displays = SDL_GetNumVideoDisplays();

    if(displays > 1)
    {
        SDL_Rect display_bounds[displays];
        for(int i = 0; i < displays; ++i)
            SDL_GetDisplayBounds(i, &display_bounds[i]);   

        // To center on second display
        x = display_bounds[1].x + ( (display_bounds[1].w / 2) - (window_width / 2) );
        y = display_bounds[1].y + ( (display_bounds[1].h / 2) - (window_height / 2) );
    }
    else
    {
        // x = SDL_WINDOWPOS_UNDEFINED;
        // y = SDL_WINDOWPOS_UNDEFINED;
        x = SDL_WINDOWPOS_CENTERED;
        y = SDL_WINDOWPOS_CENTERED;   
    }    
}

// -----------------------------------------------------------------------------------------
internal sdl_window_dimension SDL_Get_Window_Size(SDL_Window *window)
{
    int width;
    int height;
    SDL_GetWindowSize(window, &width, &height);

    return { width, height };
}

#if 0    // @STUDY
// -----------------------------------------------------------------------------------------
internal bool Allocate_Buffer(Buffer *buffer, int bytes_per_pixels, int width, int height)
{
    bool success = false;
    
    buffer->pixels = (u32 *)malloc(bytes_per_pixels * width * height);
    if(buffer->pixels != NULL)        
    {
        success = true;
        buffer->width = width;
        buffer->height = height;
        buffer->bytes_per_pixels = bytes_per_pixels;
        buffer->pitch = width * bytes_per_pixels;
        memset(buffer->pixels, 0, bytes_per_pixels * width * height);
    }
    
    return success;
}


// -----------------------------------------------------------------------------------------
internal void Reallocate_Buffer(Buffer *buffer, int w, int h)
{
    memset(buffer->pixels, 0, buffer->width * buffer->height * buffer->bytes_per_pixels);
    
    buffer->width  = w;
    buffer->height = h;
    buffer->pitch  = w * buffer->bytes_per_pixels;

    buffer->pixels = realloc(buffer->pixels, w * h * buffer->bytes_per_pixels);
    assert(buffer->pixels);
    memset(buffer->pixels, 0, w * h * buffer->bytes_per_pixels);
}
#endif    // @STUDY

// -----------------------------------------------------------------------------------------
internal SDL_Surface* Resize_Surface(SDL_Surface **surface, int w, int h)
{
    SDL_FreeSurface(*surface);
    return Create_Surface(w, h);
}

// -----------------------------------------------------------------------------------------
internal void SDL_Resize_Window(Rendering_Context *rendering_ctx, int w, int h)
{
    // @IMPORTANT: On resize the software renderer(back_buffer) is uninitialize.
    SDL_FreeSurface(rendering_ctx->back_buffer);
    SDL_SetWindowSize(rendering_ctx->window, w, h);

    int x, y;
    Center_Window_On_N_Displays(w, h, x, y);
    SDL_SetWindowPosition(rendering_ctx->window, x, y);
}

// -----------------------------------------------------------------------------------------
// The bmp image must be 32 bits color.
// Take a color filter as parameter, NULL to use default.
internal SDL_Surface* Load_Bitmap(const char *file, rgb_color *filter)
{
    SDL_Surface* output;
    
    SDL_Surface* temp_surface = SDL_LoadBMP(file);
    if(temp_surface)
    {
        SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
        output = SDL_ConvertSurface(temp_surface, format, 0);
        SDL_FreeFormat(format);
        SDL_FreeSurface(temp_surface);
        if(filter)
        {
            u32* src_row = (u32 *)output->pixels;
            for(int y = 0; y < output->h; ++y)
            {
                u32* current_pixel = src_row;
                for(int x = 0; x < output->w; ++x)
                {
                    if(*current_pixel == 0)
                    {
                        current_pixel++;
                    }
                    else
                    {
                        u8 a = (u8)((*current_pixel >> 24) * filter->a);
                        u8 r = (u8)((*current_pixel >> 16) * filter->r);
                        u8 g = (u8)((*current_pixel >>  8) * filter->g);
                        u8 b = (u8)((*current_pixel >>  0) * filter->b);
                        *current_pixel++ = ( (a << 24) | (r << 16) | (g << 8) | (b << 0) );
                    }
                }
                src_row += output->w;    // not the pitch already 32 bits(4 bytes)
            }
        }
    }  
    
    return output;
}



#if 0    // @STUDY
// -----------------------------------------------------------------------------------------
internal void Make_String_From_Int(Assets *assets, SDL_Surface *dest_surface, int number_to_stringify,
                                   int x, int y, int display_width, int display_height)
{
    memset(assets->temp_string->pixels, 0, assets->temp_string->format->BytesPerPixel * assets->temp_string->w * assets->temp_string->h);
    
    char str[10];
    sprintf(str, "%d", number_to_stringify);

    int length = 0;
    while(str[length] != '\0') ++length;

    int width = 0, max_height = 0, start = 0;
    for(int i = 0; i < length; ++i)
    {
        width += assets->font_asset[str[i] - 48]->w;              // ascii hack
        if(assets->font_asset[str[i] - 48]->h > max_height)
            max_height = assets->font_asset[str[i] - 48]->h;

        SDL_Rect font_rect = { start, 0, assets->font_asset[str[i] - 48]->w, assets->font_asset[str[i] - 48]->h };
        Bit_Blt(assets->font_asset[str[i] - 48], NULL, assets->temp_string, &font_rect);

        start += assets->font_asset[str[i] - 48]->w;        
    }  

    SDL_Rect temp_string_rect = { 0, 0, width, max_height };
    SDL_Rect dest_buffer_rect = { x, y, display_width, display_height };
    Bit_Blt(assets->temp_string, &temp_string_rect, dest_surface, &dest_buffer_rect);
}
#endif    // @STUDY

// -----------------------------------------------------------------------------------------
internal void Get_String_Size(char text[], int scale, int *width, int *height)
{
    int t = 0, max_width = 0, num_lines = 1, current_width = 0;
    
    while(text[t])
    {
        if(!((text[t] - '\n') == 0))  current_width++;
        else
        {
            num_lines++;
            if (current_width > max_width) max_width = current_width;
            
            current_width = 0;
        }        
        ++t;
    }

    if (current_width > max_width) max_width = current_width;

    *width = max_width * scale;
    *height = num_lines * scale;
}

// -----------------------------------------------------------------------------------------
internal void Make_String(SDL_Surface *dest_surface, SDL_Surface *font_surface,
                          char text[], int scale, int px, int py, int width, int height)
{    
    SDL_Surface* temp_surface = Create_Surface(width, height);
    int x = 0; int y = 0;
    int t = 0;
    int space_btw = (scale * 75) / 100;  // 3/4 of the scale looks good.
    while(text[t])
    {
        if(!( (text[t] - '\n') == 0 ))
        {
            int code = text[t] - 32;
            SDL_Rect src = { (code % 16) * 16, (code / 16) * 16, 16, 16 };
            SDL_Rect dest = { x, y, scale, scale }; 

            Bit_Blt(font_surface, &src, temp_surface, &dest);
            
            x += space_btw; // NOTE: The letter are too spaced out.
        }
        else
        {
            x = 0;
            y += scale;
        }
        ++t;
    }

    SDL_Rect dest_rect = { px, py, width, height };
    Bit_Blt(temp_surface, NULL, dest_surface, &dest_rect);
    SDL_FreeSurface(temp_surface);
}


// -----------------------------------------------------------------------------------------
internal void Load_Font_Asset(Assets *assets, rgb_color *filter)
{
    assets->font_bitmap               = Load_Bitmap("../data/Font/font_16x16.bmp", filter);
    
    assets->font_asset[ZERO]          = Load_Bitmap("../data/Font/0_.bmp", filter);
    assets->font_asset[ONE]           = Load_Bitmap("../data/Font/1_.bmp", filter);
    assets->font_asset[TWO]           = Load_Bitmap("../data/Font/2_.bmp", filter);
    assets->font_asset[THREE]         = Load_Bitmap("../data/Font/3_.bmp", filter);
    assets->font_asset[FOUR]          = Load_Bitmap("../data/Font/4_.bmp", filter);
    assets->font_asset[FIVE]          = Load_Bitmap("../data/Font/5_.bmp", filter);
    assets->font_asset[SIX]           = Load_Bitmap("../data/Font/6_.bmp", filter);
    assets->font_asset[SEVEN]         = Load_Bitmap("../data/Font/7_.bmp", filter);
    assets->font_asset[EIGHT]         = Load_Bitmap("../data/Font/8_.bmp", filter);
    assets->font_asset[NINE]          = Load_Bitmap("../data/Font/9_.bmp", filter);

    // assets->font_asset[BOMB]          = Load_Bitmap("../data/Font/B_.bmp", filter);
    // assets->font_asset[FLAG]          = Load_Bitmap("../data/Font/F_.bmp", filter);
    assets->font_asset[QUESTION_MARK] = Load_Bitmap("../data/Font/?_.bmp", filter);
}

// -----------------------------------------------------------------------------------------
internal int SDL_Get_Window_Refresh_Rate(SDL_Window *window)
{
    SDL_DisplayMode mode;
    int display_index = SDL_GetWindowDisplayIndex(window);
    // If we can't find the refresh rate, we'll return this:
    int default_refresh_rate = 30;
    if (SDL_GetDesktopDisplayMode(display_index, &mode) != 0)
    {
        return default_refresh_rate;
    }
    if (mode.refresh_rate == 0)
    {
        return default_refresh_rate;
    }
    return mode.refresh_rate;
}

// -----------------------------------------------------------------------------------------
internal float SDL_Get_Seconds_Elapsed(u64 old_counter, u64 current_counter)
{
    return ((float)(current_counter - old_counter) / (float)(SDL_GetPerformanceFrequency()));
}

// -----------------------------------------------------------------------------------------
internal bool Handle_Event(SDL_Event *event, Rendering_Context *rendering_ctx, Game_State *game_state, UI_Context *ui_ctx)
{
    bool should_quit = false;
    switch (event->type) {
        case SDL_QUIT:
        {
            should_quit = true;
            printf("SDL_QUIT\n");
        }break;
                    
        case SDL_WINDOWEVENT:
        {
            switch(event->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    rendering_ctx->back_buffer = SDL_GetWindowSurface(rendering_ctx->window);
                    printf("SDL_WINDOWEVENT_SIZE_CHANGED\n");
                } break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {     printf("SDL_WINDOWEVENT_FOCUS_GAINED\n"); } break;                            
                case SDL_WINDOWEVENT_EXPOSED:
                {     printf("SDL_WINDOWEVENT_EXPOSED\n");      } break;                            
            }
        } break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: 
        {
            ui_ctx->mouse.left_button.is_down = (event->button.state == SDL_PRESSED) && (event->button.button == SDL_BUTTON_LEFT);
            ui_ctx->mouse.left_button.release = (event->button.state == SDL_RELEASED) && (event->button.button == SDL_BUTTON_LEFT);

            ui_ctx->mouse.right_button.is_down = (event->button.state == SDL_PRESSED) && (event->button.button == SDL_BUTTON_RIGHT);
            ui_ctx->mouse.right_button.release = (event->button.state == SDL_RELEASED) && (event->button.button == SDL_BUTTON_RIGHT);            
        } break;
        
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_Keycode key_code = event->key.keysym.sym;
            bool is_down = (event->key.state == SDL_PRESSED);
            bool was_down = false;
            if (event->key.state == SDL_RELEASED)
                was_down = true;
            else if (event->key.repeat != 0)
                was_down = true;

            bool ctrl_key_was_down = (event->key.keysym.mod & KMOD_LCTRL);
            
            // NOTE: In the windows version, we used "if (is_down != was_down)"
            // to detect key repeats. SDL has the 'repeat' value, though,
            // which we'll use.
            if (event->key.repeat == 0)
            {
                if(key_code == SDLK_w)          {}
                else if(key_code == SDLK_a)     {}                                
                else if(key_code == SDLK_s)     {}
                else if(key_code == SDLK_d)     {}
                else if(key_code == SDLK_q)     {}
                else if(key_code == SDLK_e)     {}
                else if(key_code == SDLK_UP)    {}
                else if(key_code == SDLK_LEFT)  {}
                else if(key_code == SDLK_DOWN)  {}
                else if(key_code == SDLK_RIGHT) {}
                else if(key_code == SDLK_ESCAPE){}
                else if(key_code == SDLK_SPACE) {}
                
                else if(key_code == SDLK_p)
                {
                    if(ui_ctx->hot)
                        printf("pos x = %d, pos y = %d\n", ui_ctx->hot->rect.x, ui_ctx->hot->rect.y);
                }

                else if(key_code == SDLK_b)
                {
                    if(game_state->game_over) game_state->game_over = false;
                    else game_state->game_over = true;
                }

                else if(key_code == SDLK_s && ctrl_key_was_down && was_down)                    
                {
#if 0
                    UI_Save(ui_ctx);
#endif
                }

            }
        } break;
    }
    return should_quit;
}

// -----------------------------------------------------------------------------------------
inline void SDL_Update_Window(Rendering_Context *rendering_ctx)
{
    SDL_UpdateWindowSurface(rendering_ctx->window);
}

// -----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
    {        
        u64 perf_count_frequency = SDL_GetPerformanceFrequency();
        info_fps frame_info = {};
        
        // Window position and dimension
        // int window_width  = 1152;    // 36 * 32
        // int window_height = 768;     // 24 * 32
        // int window_width  = 1130;
        // int window_height = 670;
        int x, y;
        Center_Window_On_N_Displays(window_width, window_height, x, y);           

        Rendering_Context rendering_ctx;
        rendering_ctx.window = SDL_CreateWindow("Minesweeper",
                                              x,
                                              y,
                                              window_width,
                                              window_height,
                                              0
                                              /*SDL_WINDOW_RESIZABLE*/);

        if(rendering_ctx.window)
        {
#if FORCE_HZ_30
            int game_update_hz = 45;
            printf("Refresh rate is forced to %i Hz\n", game_update_hz);
#else
            int game_update_hz = SDL_Get_Window_Refresh_Rate(rendering_ctx.window);
            printf("Refresh rate is %d Hz\n", SDL_Get_Window_Refresh_Rate(rendering_ctx.window));
#endif
            float target_second_per_frame = 1.0f / (float)game_update_hz;       

            rendering_ctx.back_buffer = SDL_GetWindowSurface(rendering_ctx.window);
            if(rendering_ctx.back_buffer)
            {                
                Assets assets;
                rgb_color default_font_color = { 0, 0, 0, 1.0f };
                Load_Font_Asset(&assets, &default_font_color);

                // @TODO: Is rand() enough?
                srand(time(nullptr));
                                
                // ----------------------------------------------------------------------

                Game_State game_state = {};
                game_state.running = true;
                game_state.mode = MENU;
                game_state.screen_size.w = window_width;
                game_state.screen_size.h = window_height;
                game_state.cells_to_test = new_int_array(256);
                game_state.screen_padding = { 50, 50 };

                game_state.parkour_load_new_level = true;
                Init_Parkour_Data(&game_state.parkour_data);

                bool success_init_game_assets = Init_Game_Assets(&assets, &game_state);
                
                // ===================================================================
                
#if 0 // @TODO: Remove when done.
                Make_Grid_BMP(9, 9, 32, 32);
#endif

#if 0
                Buffer bmp_buffer;
                SDL_Surface* test_bmp;
                if(Load_BMP("../data/bmp_test.bmp", &bmp_buffer))
                {
                    test_bmp = Create_Surface(bmp_buffer.w, bmp_buffer.h);
                    if(test_bmp)
                        memcpy(test_bmp->pixels, bmp_buffer.pixels, bmp_buffer.w * bmp_buffer.h * 4);  
                }
#endif                               

                // ===================================================================

                UI_Context ui_ctx = {};
                Init_UI_Context(&game_state, &ui_ctx, &assets);                

                
                Update_And_Render(&rendering_ctx, &assets, &game_state, &ui_ctx);

                Font_Data font_data;
                char path_to_font[] = "../data/Font/font_data_32.dat";
                extract_font_data(&font_data, path_to_font);
                
                // -----------------------------------------------------------------------
                SDL_Event event;

                uint64_t last_counter = SDL_GetPerformanceCounter();
                uint64_t last_cycle_count = _rdtsc();
                // ------------------------------------------------------------------------
                while(game_state.running)
                {                    
                    while (SDL_PollEvent(&event))
                    {
                        // @NOTE: Instead of returning a bool, we could return a struct with a bit more
                        // information about what happened during event poll...
                        if(Handle_Event(&event, &rendering_ctx, &game_state, &ui_ctx))
                        {
                            game_state.running = false;
                        }
                    }                    
                    // Process_Keyboard_State();
                    // const Uint8 *state = SDL_GetKeyboardState(NULL);
                    // ----------------------------------------------------------------

                    SDL_GetMouseState(&ui_ctx.mouse.x, &ui_ctx.mouse.y);
                    Game_Update_And_Render(&rendering_ctx, &game_state, &assets, &ui_ctx);

                    // -----------------------------------------------------------------
                    uint64_t work_counter = SDL_GetPerformanceCounter();
                    float work_second_elapsed = SDL_Get_Seconds_Elapsed(last_counter, work_counter);
                        
                    float second_elapsed_for_frame = work_second_elapsed;
                    if(second_elapsed_for_frame < target_second_per_frame)
                    {
                        s32 sleep_ms = (s32)(1000.0f * (target_second_per_frame - second_elapsed_for_frame));
                        if(sleep_ms > 0)
                        {
                            SDL_Delay(sleep_ms);
                        }

                        // @TODO: Assert elapsed_time < target_second_per_frame;
                        // assert(test_second_elapsed_for_frame < target_second_per_frame);
                        float test_second_elapsed_for_frame = SDL_Get_Seconds_Elapsed(last_counter, SDL_GetPerformanceCounter());

                        while(test_second_elapsed_for_frame < target_second_per_frame)
                        {
                            test_second_elapsed_for_frame = SDL_Get_Seconds_Elapsed(last_counter, SDL_GetPerformanceCounter());
                        }
                    }
                    else
                    {
                        // @TODO: Missed frame rate
                        // @TODO: logging
#if INFO_PRINT_FPS
                        printf("Missed frame rate! time elapsed = %f\n", second_elapsed_for_frame);
#endif
                    }
                        
                    // -----------------------------------------------------------------
                    uint64_t end_counter = SDL_GetPerformanceCounter();
                    SDL_Update_Window(&rendering_ctx);
                    // -----------------------------------------------------------------                        
#if INFO_PRINT_FPS
                    uint64_t end_cycle_count = _rdtsc();
                    uint64_t cycles_elapsed = end_cycle_count - last_cycle_count;
                    last_cycle_count = end_cycle_count;

                    
                    frame_info.ms_per_frame = 1000.0f * SDL_Get_Seconds_Elapsed(last_counter, end_counter);
                    frame_info.fps = 1.0f / ( frame_info.ms_per_frame / 1000.0f);   
                    frame_info.mcpf = ((double)cycles_elapsed / (1000.0f * 1000.0f));                        

#if 0
                    printf("frame_info.ms_per_frame: %f\n", frame_info.ms_per_frame);
                    printf("frame_info.fps: %f\n", frame_info.fps);
                    printf("frame_info.mcpf: %f\n", frame_info.mcpf);
#endif
#endif
                        
                    last_counter = end_counter;                        
                }              
            }
            else
            {
                printf("Could not allocate back_buffer!");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            printf("Coundn't open window.\n");
        }
    }
    else
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
    }

    SDL_Quit();
    return 0;
}
