#include "ui.h"

global_variable SDL_Rect SMALL_RECT_DEFAULT = { 0, 0, 50, 50 };
global_variable SDL_Rect SMALL_RECT_HOT = { 0, 50, 50, 50 };
global_variable SDL_Rect SMALL_RECT_INTERACTING = { 0, 100, 50, 50 };

global_variable SDL_Rect LARGE_BUTTON_DEFAULT = { 0, 0, 150, 50 };
global_variable SDL_Rect LARGE_BUTTON_HOT = { 0, 50, 150, 50 };
global_variable SDL_Rect LARGE_BUTTON_INTERACTING = { 0, 100, 150, 50 };


// -----------------------------------------------------------------------------------------
internal void UI_Draw(Assets *assets, UI_Context *ui_ctx, Game_State *game_state, UI_Element *element)
{
    int color = colors::DEFAULT;
    if(ui_ctx->next_hot == element)
        color = colors::HOT;
    if(ui_ctx->interacting == element)
        color = colors::INTERACTING;
    Draw_Full_Rect_At(assets->ui_layer->pixels, element->rect.x, element->rect.y, element->rect.w, element->rect.h,
                      assets->ui_layer->pitch, assets->ui_layer->format->BytesPerPixel, game_colors[color]);

    char temp[16];
    int text_width, text_height;
    if(element->type == TIMER)
    {
        sprintf(temp, "timer: %d", (int)game_state->timer);
        Get_String_Size(temp, 16, &text_width, &text_height);
        Make_String(assets->ui_layer, assets->font_bitmap, temp,
                    16,
                    element->rect.x,
                    element->rect.y + (element->rect.h/4),
                    text_width, text_height);
    }

    if(element->type == BOMB_COUNTER)
    {
        sprintf(temp, "bombs: %d", (int)game_state->bomb_count);
        Get_String_Size(temp, 16, &text_width, &text_height);
        Make_String(assets->ui_layer, assets->font_bitmap, temp,
                    16,
                    element->rect.x,
                    element->rect.y + (element->rect.h/4),
                    text_width, text_height);
    }
}

// -----------------------------------------------------------------------------------------
internal UI_Element UI_Add_Element(UI_Type type, void(*element_fn)(Game_State*, Rendering_Context*, Assets*, UI_Context*),
                                   SDL_Rect(*get_rect_fn)(v2))
{
    UI_Element new_element = {};
    new_element.type = type;
    new_element.fn = element_fn;
    new_element.get_rect_fn = get_rect_fn;

    return new_element;
}

// -----------------------------------------------------------------------------------------
internal UI_Element UI_Add_Element(UI_Type type, void(*element_fn)(Game_State*, Rendering_Context*, Assets*, UI_Context*),
                                   int x, int y, int w, int h)
{
    UI_Element new_element = {};
    new_element.type = type;
    new_element.fn = element_fn;
    new_element.rect = { x, y, w, h };

    return new_element;
}


// -----------------------------------------------------------------------------------------
internal UI_Element UI_Add_Level(UI_Type type, int level, void(*element_fn)(Game_State*, int), SDL_Rect rect)
{
    UI_Element new_element = {};
    new_element.type = type;
    new_element.level_to_select = level;
    new_element.select_lvl = element_fn;
    new_element.rect = rect;

    return new_element;
}

// -----------------------------------------------------------------------------------------
internal void Init_UI_Context(Game_State *game_state, UI_Context *ui_ctx, Assets *assets)
{
    bool success_ui_layer = false;
    assets->ui_layer = Create_Surface(game_state->screen_size.w, game_state->screen_size.h);
    if(assets->ui_layer) success_ui_layer = true;

    v2 size = game_state->screen_size;
    int w, h;
    // -------------------------------------------------------
    // Menu
    ui_ctx->menu_ui_count = 3;
    ui_ctx->menu = (array_of_ui_element)malloc(sizeof(UI_Element) * ui_ctx->menu_ui_count);
    w = 150; h = 50;
    ui_ctx->menu[0] = UI_Add_Element(BUTTON, Select_Classic, size.w - (int)(w*2), (size.h / 3) - (h+10), w, h);
    ui_ctx->menu[0].btn_bmp = &assets->btn_classic;
    
#if 0 // @NOTE: Given up on mined out mode.
    ui_ctx->menu[1] = UI_Add_Element(BUTTON, Select_Mined, size.w - (int)(w*2), (size.h / 3), w, h);
    ui_ctx->menu[1].btn_bmp = &assets->btn_mine;
#endif
    
    ui_ctx->menu[1] = UI_Add_Element(BUTTON, Select_Parkour, size.w - (int)(w*2), (size.h / 3), w, h);
    ui_ctx->menu[1].btn_bmp = &assets->btn_parkour;
    ui_ctx->menu[2] = UI_Add_Element(BUTTON, Quit, size.w - (int)(w*2), (size.h / 3) + (h*6), w, h);
    ui_ctx->menu[2].btn_bmp = &assets->btn_exit;
        
    // -------------------------------------------------------
    // Select Parkour level
    ui_ctx->select_parkour_level_ui_count = 1;
    ui_ctx->select_parkour_level_ui = (array_of_ui_element)malloc(sizeof(UI_Element) * ui_ctx->select_parkour_level_ui_count);
    w = 50; h = 50;
    ui_ctx->select_parkour_level_ui[0] = UI_Add_Element(BUTTON, Menu_Up, 0, size.h - h, w, h);

    ui_ctx->select_level_ui = (array_of_ui_element)malloc(sizeof(UI_Element) * game_state->parkour_data.number_of_levels);
    v2 starting_points = { 300, 300 };
    for(int l = 0; l < game_state->parkour_data.number_of_levels; ++l)
    {
        int marge = (50 * l) + (50 * l);
        SDL_Rect rect = { starting_points.x + marge, starting_points.y, 50, 50 };
        ui_ctx->select_level_ui[l] = UI_Add_Level(LEVEL_BUTTON, l, Select_Level, rect);
    }
    
    // ------------------------------------------------------
    // Classic
    ui_ctx->classic_ui_count = 7;
    ui_ctx->classic = (array_of_ui_element)malloc(sizeof(UI_Element) * ui_ctx->classic_ui_count);
    w = 50; h = 50;
    ui_ctx->classic[0] = UI_Add_Element(BUTTON, New_Game, 0, 0, w, h);
    ui_ctx->classic[1] = UI_Add_Element(BUTTON, Menu_Up, 0, size.h - h, w, h);
    ui_ctx->classic[1].btn_bmp = &assets->btn_return;
    w = 150; h = 50;
    ui_ctx->classic[2] = UI_Add_Element(TIMER, NULL, size.w - w, 0, w, h);
    w = 140; h = 50;
    ui_ctx->classic[3] = UI_Add_Element(BOMB_COUNTER, NULL, (size.w/2) - (w/2), 0, w, h);
    w = 50; h = 50;
    ui_ctx->classic[4] = UI_Add_Element(BUTTON, Small,  size.w - w, size.h - (250), w, h);
    ui_ctx->classic[5] = UI_Add_Element(BUTTON, Medium, size.w - w, size.h - (175), w, h);
    ui_ctx->classic[6] = UI_Add_Element(BUTTON, Large,  size.w - w, size.h - (100), w, h);

    // -----------------------------------------------------
#if 0 // @NOTE: Given up on mined out mode.
    // Mined out
    ui_ctx->mined_ui_count = 2;
    ui_ctx->mined = (array_of_ui_element)malloc(sizeof(UI_Element) * ui_ctx->mined_ui_count);
    w = 50; h = 50;
    ui_ctx->mined[0] = UI_Add_Element(BUTTON, New_Game, 0, 0, w, h);    
    ui_ctx->mined[1] = UI_Add_Element(BUTTON, Menu_Up, 0, size.h - h, w, h);
#endif

    // -----------------------------------------------------
    // Parkour time
    ui_ctx->parkour_ui_count = 5;
    ui_ctx->parkour = (array_of_ui_element)malloc(sizeof(UI_Element) * ui_ctx->parkour_ui_count);
    w = 50; h = 50;
    ui_ctx->parkour[0] = UI_Add_Element(BUTTON, New_Game, 0, 0, w, h);
    ui_ctx->parkour[1] = UI_Add_Element(BUTTON, Menu_Up, 0, size.h - h, w, h);
    w = 150; h = 50;
    ui_ctx->parkour[2] = UI_Add_Element(TIMER, NULL, size.w - w, 0, w, h);
    w = 140; h = 50;
    ui_ctx->parkour[3] = UI_Add_Element(BOMB_COUNTER, NULL, (size.w/2) - (w/2), 0, w, h);
    w = 50; h = 50;
    ui_ctx->parkour[4] = UI_Add_Element(BUTTON, Select_Parkour, size.w - w, size.h - h, w, h);

    // -----------------------------------------------------
    // Classic Win Menu
    ui_ctx->classic_win_ui_count = 2;
    ui_ctx->classic_win = (array_of_ui_element)malloc(sizeof(UI_Element) * ui_ctx->classic_win_ui_count);
    w = 100; h = 50;
    ui_ctx->classic_win[0] = UI_Add_Element(BUTTON, New_Game_Classic, (size.w/2) - (w/2), (size.h/2) + h, w, h);
    ui_ctx->classic_win[1] = UI_Add_Element(BUTTON, Menu_Up, (size.w/2) - (w/2), (size.h/2) + (h*2 + 2), w, h);
}


// -----------------------------------------------------------------------------------------
internal void UI_Update(Game_State *game_state, Rendering_Context *rendering_ctx, UI_Context *ui_ctx, Assets *assets)
{
    memset(assets->ui_layer->pixels, 0, assets->ui_layer->bytes_per_pixels * assets->ui_layer->w * assets->ui_layer->h);

    ui_ctx->next_hot = 0;

    switch(game_state->mode)
    {
        case MENU:
        {
            for(int i = 0; i < ui_ctx->menu_ui_count; ++i)
            {
                if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                                 ui_ctx->menu[i].rect.x, ui_ctx->menu[i].rect.y,
                                 ui_ctx->menu[i].rect.w, ui_ctx->menu[i].rect.h))
                {
                    ui_ctx->next_hot = &ui_ctx->menu[i];
                }            
                assert(ui_ctx->menu[i].rect.x >= 0 && ui_ctx->menu[i].rect.x < assets->ui_layer->w &&
                       ui_ctx->menu[i].rect.y >= 0 && ui_ctx->menu[i].rect.y < assets->ui_layer->h);

                SDL_Rect src_rect = LARGE_BUTTON_DEFAULT;               
                if(ui_ctx->next_hot == &ui_ctx->menu[i])    src_rect = LARGE_BUTTON_HOT;
                if(ui_ctx->interacting == &ui_ctx->menu[i]) src_rect = LARGE_BUTTON_INTERACTING;
                
                Bit_Blt(*ui_ctx->menu[i].btn_bmp, &src_rect, assets->ui_layer, &ui_ctx->menu[i].rect);
            }
        } break;

        case SELECT_PARKOUR_LEVEL:
        {
            for(int i = 0; i < ui_ctx->select_parkour_level_ui_count; ++i)
            {
                if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                                 ui_ctx->select_parkour_level_ui[i].rect.x, ui_ctx->select_parkour_level_ui[i].rect.y,
                                 ui_ctx->select_parkour_level_ui[i].rect.w, ui_ctx->select_parkour_level_ui[i].rect.h))
                {
                    ui_ctx->next_hot = &ui_ctx->select_parkour_level_ui[i];
                }            
                
                UI_Draw(assets, ui_ctx, game_state, &ui_ctx->select_parkour_level_ui[i]);
            }

            for(int l = 0; l < game_state->parkour_data.number_of_levels; ++l)
            {
                if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                                 ui_ctx->select_level_ui[l].rect.x, ui_ctx->select_level_ui[l].rect.y,
                                 ui_ctx->select_level_ui[l].rect.w, ui_ctx->select_level_ui[l].rect.h))
                {
                    ui_ctx->next_hot = &ui_ctx->select_level_ui[l];
                }
                
                UI_Draw(assets, ui_ctx, game_state, &ui_ctx->select_level_ui[l]);
            }                       
        } break;
        
        case CLASSIC:
        {
            for(int i = 0; i < ui_ctx->classic_ui_count; ++i)
            {
                if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                                 ui_ctx->classic[i].rect.x, ui_ctx->classic[i].rect.y,
                                 ui_ctx->classic[i].rect.w, ui_ctx->classic[i].rect.h))
                {
                    ui_ctx->next_hot = &ui_ctx->classic[i];
                }            
                
                assert(ui_ctx->classic[i].rect.x >= 0 && ui_ctx->classic[i].rect.x < assets->ui_layer->w &&
                       ui_ctx->classic[i].rect.y >= 0 && ui_ctx->classic[i].rect.y < assets->ui_layer->h);

                if(ui_ctx->classic[i].btn_bmp)
                {                                    
                    SDL_Rect src_rect = SMALL_RECT_DEFAULT;               
                    if(ui_ctx->next_hot == &ui_ctx->classic[i])    src_rect = SMALL_RECT_HOT;
                    if(ui_ctx->interacting == &ui_ctx->classic[i]) src_rect = SMALL_RECT_INTERACTING;
                
                    Bit_Blt(*ui_ctx->classic[i].btn_bmp, &src_rect, assets->ui_layer, &ui_ctx->classic[i].rect);
                }
                else
                {
                    UI_Draw(assets, ui_ctx, game_state, &ui_ctx->classic[i]);   
                }
            }
        } break;

        case CLASSIC_WIN:
        {
            for(int i = 0; i < ui_ctx->classic_win_ui_count; ++i)
            {
                if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                                 ui_ctx->classic_win[i].rect.x, ui_ctx->classic_win[i].rect.y,
                                 ui_ctx->classic_win[i].rect.w, ui_ctx->classic_win[i].rect.h))
                {
                    ui_ctx->next_hot = &ui_ctx->classic_win[i];
                }            
                
                assert(ui_ctx->classic_win[i].rect.x >= 0 && ui_ctx->classic_win[i].rect.x < assets->ui_layer->w &&
                       ui_ctx->classic_win[i].rect.y >= 0 && ui_ctx->classic_win[i].rect.y < assets->ui_layer->h);

                UI_Draw(assets, ui_ctx, game_state, &ui_ctx->classic_win[i]);
            }
        } break;

#if 0 // @NOTE: Given up on mined out mode.
        case MINED:
        {            
            for(int i = 0; i < ui_ctx->mined_ui_count; ++i)
            {
                if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                                 ui_ctx->mined[i].rect.x, ui_ctx->mined[i].rect.y,
                                 ui_ctx->mined[i].rect.w, ui_ctx->mined[i].rect.h))
                {
                    ui_ctx->next_hot = &ui_ctx->mined[i];
                }
                
                assert(ui_ctx->mined[i].rect.x >= 0 && ui_ctx->mined[i].rect.x < assets->ui_layer->w &&
                       ui_ctx->mined[i].rect.y >= 0 && ui_ctx->mined[i].rect.y < assets->ui_layer->h);

                UI_Draw(assets, ui_ctx, game_state, &ui_ctx->mined[i]);
            }            
        } break;
#endif

        case PARKOUR:
        {            
            for(int i = 0; i < ui_ctx->parkour_ui_count; ++i)
            {
                if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                                 ui_ctx->parkour[i].rect.x, ui_ctx->parkour[i].rect.y,
                                 ui_ctx->parkour[i].rect.w, ui_ctx->parkour[i].rect.h))
                {
                    ui_ctx->next_hot = &ui_ctx->parkour[i];
                }            
                
                assert(ui_ctx->parkour[i].rect.x >= 0 && ui_ctx->parkour[i].rect.x < assets->ui_layer->w &&
                       ui_ctx->parkour[i].rect.y >= 0 && ui_ctx->parkour[i].rect.y < assets->ui_layer->h);

                UI_Draw(assets, ui_ctx, game_state, &ui_ctx->parkour[i]);
            }            
        } break;
    }


    if(ui_ctx->interacting) 
    {
        if(ui_ctx->mouse.left_button.release)
        {
            if(ui_ctx->interacting->type == BUTTON)
                ui_ctx->interacting->fn(game_state, rendering_ctx, assets, ui_ctx);
            if(ui_ctx->interacting->type == LEVEL_BUTTON)
                ui_ctx->interacting->select_lvl(game_state, ui_ctx->interacting->level_to_select);

            ui_ctx->interacting = 0;
        }
        else
        {
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            // @NOTE: sdl doesn't take into account keyboard remap.
            if(state[SDL_SCANCODE_LCTRL])
            {
                int drag_x = ui_ctx->mouse.x - (ui_ctx->interacting->rect.x + (ui_ctx->interacting->rect.w / 2));
                int drag_y = ui_ctx->mouse.y - (ui_ctx->interacting->rect.y + (ui_ctx->interacting->rect.h / 2));

                ui_ctx->interacting->rect.x += drag_x;
                ui_ctx->interacting->rect.y += drag_y;    
            }
        }
    }    
    else
    {
        ui_ctx->hot = ui_ctx->next_hot;
        if(!ui_ctx->mouse.left_button.was_down && ui_ctx->mouse.left_button.is_down)
        {
            ui_ctx->interacting = ui_ctx->hot;           
        }
    }
}

