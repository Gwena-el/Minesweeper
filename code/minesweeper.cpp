 #include "minesweeper.h"

global_variable rgb_color game_colors[colors::MAX_COLOR];

#include "grid.cpp"
#include "ui.cpp"
#include "debug_and_make_game.cpp"


#define BEST_TIME_PATH "../data/best_time.txt"


// #################################### UTILITY ############################################
// -----------------------------------------------------------------------------------------
// @NOTE: Min and Max are included.
internal int Get_Random_Number(int min, int max)
{
    double fraction { 1.0 / (RAND_MAX + 1.0) };
    return min + (int)((max - min + 1) * (rand() * fraction));
}

// -----------------------------------------------------------------------------------------
//internal void Extract_Color_Palette(Game_State *game_state, const char *color_dat)
internal void Extract_Color_Palette(rgb_color game_colors[], const char *color_dat)
{
    FILE* file = fopen(color_dat, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        u32* file_buffer = (u32*)malloc(file_size);        
        if(file_buffer)
        {          
            fread(file_buffer, 4, file_size / 4, file);   // read each entry as 32 bits;
            fclose(file);
            
            u32* current = file_buffer;
            int a, r, g, b;
            for(int i = 0; i < colors::MAX_COLOR; ++i)
            {
                a = (int)((u8)(*current >> 24));
                r = (int)((u8)(*current >> 16));
                g = (int)((u8)(*current >>  8));
                b = (int)((u8)(*current >>  0));
                game_colors[i] = rgb_to_float(r, g, b, a);

                current++;
            }
        }

        free(file_buffer);        
    }
}

// -----------------------------------------------------------------------------------------
struct return_time
{
    u32 small;
    u32 medium;
    u32 large;
};
internal return_time Load_Best_Time(const char *best_time_file)
{
    return_time return_time = {};
    FILE *file = fopen(best_time_file, "r");
    if(file)
    {
        char temp1[16], temp2[16], temp3[16];
        fscanf(file, "%s %s %s", temp1, temp2, temp3);
        return_time.small  = atoi(temp1);
        return_time.medium = atoi(temp2);
        return_time.large  = atoi(temp3);
        fclose(file);
    }        
        
    return return_time;
}

// -----------------------------------------------------------------------------------------
inline bool Point_In_Rect(int x, int y, int rx, int ry, int rw, int rh)
{
    if((x > rx) && (x < rx + rw) && (y > ry) && (y < ry + rh))
        return true;

    return false;
}

// -----------------------------------------------------------------------------------------
inline v2 Distance_2Point(int x0, int y0, int x1, int y1)
{
    return { x1 - x0, y1 - y0 };
}

// -----------------------------------------------------------------------------------------
inline int Is(u16 mask, Cell *cells, int x, int y, int nw, int nh)
{
    if( (x >= 0) && (x < nw) &&
        (y >= 0) && (y < nh) )
    {
        int i = y * nw + x;
        if(cells[i].flag & mask) return 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------------------
inline int IsNot(u16 mask, Cell *cells, int x, int y, int nw, int nh)
{
    if( (x >= 0) && (x < nw) &&
        (y >= 0) && (y < nh) )
    {
        int i = y * nw + x;
        if(!(cells[i].flag & mask)) return 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------------------
inline int cell_test(u16 mask, Cell *cells, int x, int y, int nw, int nh)
{
    if( (x >= 0) && (x < nw) &&
        (y >= 0) && (y < nh) )
    {
        int i = y * nw + x;
        if((cells[i].flag & mask) && !(cells[i].flag & MASK_TESTED))
            return 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------------------
internal int Neighbours_Check(int (*test)(u16, Cell*, int, int, int, int), u16 mask,
                              Cell *cells, int x, int y, int nw, int nh)
{
    u8 neighbours =
        test(mask, cells, x-1, y-1, nw, nh) + test(mask, cells, x  , y-1, nw, nh) + test(mask, cells, x+1, y-1, nw, nh) +
        test(mask, cells, x-1, y  , nw, nh) +                   0                 + test(mask, cells, x+1, y  , nw, nh) +
        test(mask, cells, x-1, y+1, nw, nh) + test(mask, cells, x  , y+1, nw, nh) + test(mask, cells, x+1, y+1, nw, nh);

    return neighbours;
}

// -----------------------------------------------------------------------------------------
internal void Push_And_Mark_Cell(Game_State *game_state, int x, int y, int nw)
{
    push_back(&game_state->cells_to_test, y * nw + x);
    game_state->current_grid->cells[y * nw + x].flag |= MASK_TESTED;
}

// -----------------------------------------------------------------------------------------
internal void Push_Cell_to_Test(Game_State *game_state, int x, int y, int nw, int nh)
{
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x-1, y-1, nw, nh))
        Push_And_Mark_Cell(game_state, x-1, y-1, nw);
                    
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x, y-1, nw, nh))
        Push_And_Mark_Cell(game_state, x, y-1, nw);
                    
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x+1, y-1, nw, nh))
        Push_And_Mark_Cell(game_state, x+1, y-1, nw);
                    
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x-1, y  , nw, nh))
        Push_And_Mark_Cell(game_state, x-1, y, nw);
                    
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x+1, y  , nw, nh))
        Push_And_Mark_Cell(game_state, x+1, y, nw);
                    
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x-1, y+1, nw, nh))
        Push_And_Mark_Cell(game_state, x-1, y+1, nw);
                    
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x  , y+1, nw, nh))
        Push_And_Mark_Cell(game_state, x, y+1, nw);
                    
    if(cell_test(MASK_COVER | MASK_FLAG | MASK_QUESTION_MARK, game_state->current_grid->cells, x+1, y+1, nw, nh))
        Push_And_Mark_Cell(game_state, x+1, y+1, nw);
}

// -----------------------------------------------------------------------------------------
internal bool Get_Neighbours_Cover(Grid *grid, Cell *cells, int x, int y)
{
    bool need_clearing = false;
    u8 neighbours_covered = Neighbours_Check(Is, MASK_COVER, cells, x, y, grid->n_cells.w, grid->n_cells.h);
    if(neighbours_covered) need_clearing = true;

    return need_clearing;
}

inline v2 Grid_Size(int nw, int nh, int cw, int ch) 
{
    return { nw * cw, nh * ch };
}

// -----------------------------------------------------------------------------------------
inline v2 Center_Pos(v2 grid_size, v2 screen_size)
{
    assert(grid_size.w < screen_size.w && grid_size.h < screen_size.h);
    v2 grid_pos =  {
        (screen_size.w/2) - (grid_size.w/2),
        (screen_size.h/2) - (grid_size.h/2)
    };

    return grid_pos;
}

// #########################################################################################


// ##################################### INIT ##############################################

#if USING_SDL_SURFACE
// -----------------------------------------------------------------------------------------
internal bool Init_Game_Assets(Assets *assets, Game_State *game_state)
{
    bool success = false;

    Extract_Color_Palette(game_colors, "../data/color.dat");
    return_time temp = Load_Best_Time(BEST_TIME_PATH);
    game_state->recorded_times[Classic_Grid::SMALL]  = temp.small;
    game_state->recorded_times[Classic_Grid::MEDIUM] = temp.medium;
    game_state->recorded_times[Classic_Grid::LARGE]  = temp.large;
    
    bool success_grid_buffer = false;
    assets->grid_buffer = Create_Surface(game_state->screen_size.w, game_state->screen_size.h);
    if(assets->grid_buffer) success_grid_buffer = true;

    bool success_menu_background = false;
    assets->menu_background = Load_Bitmap("../data/menu_background.bmp", 0);
    if(assets->menu_background) success_menu_background = true;    

    bool success_temp_string_buffer = false;
    assets->temp_string = Create_Surface(game_state->screen_size.w, game_state->screen_size.h);
    if(assets->temp_string) success_temp_string_buffer = true;
    
    bool load_bomb = false;
    assets->bomb =  Load_Bitmap("../data/Bomb-bis.bmp", 0);
    if(assets->bomb) load_bomb = true;

    bool load_bomb_explode = false;
    assets->bomb_explode = Load_Bitmap("../data/Bomb_Explode-bis.bmp", &game_colors[colors::EXPLODE]);
    if(assets->bomb_explode) load_bomb_explode = true;

    bool load_flag = false;
    assets->flag = Load_Bitmap("../data/Flag.bmp", 0);
    if(assets->flag) load_flag = true;

    bool load_cover = false;
    assets->cover = Load_Bitmap("../data/Cover_Darker.bmp", 0);
    if(assets->cover) load_cover = true;


    bool load_btn_classic = false;
    assets->btn_classic = Load_Bitmap("../data/Classic_Btn.bmp", 0);
    if(assets->btn_classic) load_btn_classic = true;

    bool load_btn_mine = false;
    assets->btn_mine = Load_Bitmap("../data/Mine_Btn.bmp", 0);
    if(assets->btn_mine) load_btn_mine = true;

    bool load_btn_parkour = false;
    assets->btn_parkour = Load_Bitmap("../data/Parkour_Btn.bmp", 0);
    if(assets->btn_parkour) load_btn_parkour = true;

    bool load_btn_exit = false;
    assets->btn_exit = Load_Bitmap("../data/Exit_Btn.bmp", 0);
    if(assets->btn_exit) load_btn_exit = true;
    
    bool load_btn_return = false;
    assets->btn_return = Load_Bitmap("../data/Return_Btn.bmp", 0);
    if(assets->btn_return) load_btn_return = true;
    
    
        
    if(success_grid_buffer && load_cover && load_bomb && load_flag)
        success = true;

    return success;
}
#else
internal bool Init_Game_Assets(Assets *assets, Game_State *game_state)
{
    bool success = false;

    Extract_Color_Palette(game_colors, "../data/color.dat");
    return_time temp = Load_Best_Time(BEST_TIME_PATH);
    game_state->recorded_times[Classic_Grid::SMALL]  = temp.small;
    game_state->recorded_times[Classic_Grid::MEDIUM] = temp.medium;
    game_state->recorded_times[Classic_Grid::LARGE]  = temp.large;
    
    Create_BMP_Buffer(assets->grid_buffer, game_state->screen_size.w, game_state->screen_size.h);
    Create_BMP_Buffer(assets->temp_string, game_state->screen_size.w, game_state->screen_size.h);

    bool success_menu_background = Load_BMP("../data/menu_background.bmp", assets->menu_background, 0);

    bool load_bomb = Load_BMP("../data/Bomb-bis.bmp", assets->bomb, 0);

    bool load_bomb_explode = Load_BMP("../data/Bomb_Explode-bis.bmp", assets->bomb_explode, &game_colors[colors::EXPLODE]);

    bool load_flag = Load_BMP("../data/Flag.bmp", assets->flag, 0);

    bool load_cover = Load_BMP("../data/Cover_Darker.bmp", assets->cover, 0);

    bool load_btn_classic = Load_BMP("../data/Classic_Btn.bmp", assets->btn_classic, 0);
    
    //bool load_btn_mine = Load_BMP("../data/Mine_Btn.bmp", assets->btn_mine, 0);
    
    bool load_btn_parkour = Load_BMP("../data/Parkour_Btn.bmp", assets->btn_parkour, 0);

    bool load_btn_exit = Load_BMP("../data/Exit_Btn.bmp", assets->btn_exit, 0);
    
    bool load_btn_return = Load_BMP("../data/Return_Btn.bmp", assets->btn_return, 0);    
    
    if(assets->grid_buffer->pixels && assets->temp_string->pixels &&
       load_bomb && load_bomb_explode && load_flag && load_cover &&
       load_btn_classic && load_btn_parkour && load_btn_exit && load_btn_return)
        success = true;

    return success;
}
#endif

// -----------------------------------------------------------------------------------------
internal void Save_Best_Time(Game_State *game_state)
{
    FILE *file = fopen(BEST_TIME_PATH, "w");
    if(file)
    {
        char new_best_time[64];            
        int string_length;
        string_length = sprintf(new_best_time, "%u %u %u",
                                game_state->recorded_times[Classic_Grid::SMALL],
                                game_state->recorded_times[Classic_Grid::MEDIUM],
                                game_state->recorded_times[Classic_Grid::LARGE]);
        fwrite(new_best_time, 1, string_length, file);
        fclose(file);
    }    
}

// -----------------------------------------------------------------------------------------
internal void Load_Level_Data(Level_Array *array_of_levels, char filename[])
{
    FILE* file = fopen(filename, "r");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        int filesize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* buffer = (char *)malloc(sizeof(char) * filesize);
        fread(buffer, 1, filesize, file);
        fclose(file);

        int grid_count = 0, c = 0;
        int starting_char_new_grid[64];
        for(int i = 0; i < filesize; ++i)
        {
            if(buffer[i] == '\n' && buffer[i+1] == '\n')
            {
                grid_count++;
                starting_char_new_grid[c++] = i + 2;
            }
        }

        array_of_levels->grid_count =  grid_count;
        array_of_levels->level = (Grid_Data *)malloc(sizeof(Grid_Data) * array_of_levels->grid_count);

        int index, nw, nh, cw, ch, pos_x, pos_y, bombs_number;
        char *char_grid = buffer;

        for(int c = 0; c < grid_count; ++c)
        {
            sscanf(char_grid,
                   "index: %d\nnw: %d\nnh: %d\ncw: %d\nch: %d\npos_x: %d\npos_y: %d\nbombs: %d",
                   &index, &nw, &nh, &cw, &ch, &pos_x, &pos_y, &bombs_number);            

            array_of_levels->level[c] = { index, nw, nh, cw, ch, pos_x, pos_y, bombs_number };

            char_grid = &buffer[starting_char_new_grid[c]];
        }
    }
}

// -----------------------------------------------------------------------------------------
internal int Get_Number_level_File()
{
    char filename[64];
    FILE* file;
    int count = 0;
    sprintf(filename, "../data/level%d.txt", count);

    while(true)
    {
        file = fopen(filename, "r");
        if(file)
        {
            fclose(file);
            count++;
            sprintf(filename, "../data/level%d.txt", count);
        }
        else break;        
    }

    return count;
}

// -----------------------------------------------------------------------------------------
internal void Init_Parkour_Data(Parkour_Data *parkour_data)
{
    parkour_data->number_of_levels = Get_Number_level_File();
    parkour_data->array_of_levels = (Level_Array *)malloc(sizeof(Level_Array) * parkour_data->number_of_levels);

    for(int l = 0; l < parkour_data->number_of_levels; ++l)
    {
        char filename[32];
        sprintf(filename, "../data/level%d.txt", l);
        Load_Level_Data(&parkour_data->array_of_levels[l], filename);
    }            
}

// #########################################################################################


// -----------------------------------------------------------------------------------------
internal void Clear_Game_State(Game_State *game_state)
{
    Grid* current = game_state->grid;
    while(current)
    {
        memset(current->cells, 0, sizeof(Cell) * current->n_cells.h * current->n_cells.w);
        current = current->next;
    }

    clear_array(&game_state->cells_to_test);
    
    game_state->last_time = 0;
    game_state->timer = 0;
    game_state->game_over = false;
    game_state->game_win = false;
    game_state->parkour_finished = false;
    game_state->init = true;
    game_state->click_counter = 0;
    game_state->first_click_check = false;
}

// -----------------------------------------------------------------------------------------
internal GAME_FUNCTION(New_Game_Classic)
{
    game_state->running_game = CLASSIC;
    if(game_state->mode == CLASSIC_WIN) game_state->mode = CLASSIC;

    if(game_state->grid_count == 0) Init_Grids(game_state);    
    if(game_state->grid_count > 1)
    {
        Keep_Main_Grid_Only(&game_state->grid);
        game_state->current_grid = game_state->grid;
        game_state->grid_count = 1;
    }    

    int nw, nh, cw, ch, bombs_number;
    switch(game_state->classic_grid_size)
    {
        case Classic_Grid::SMALL:
            nw = 9; nh = 9; cw = 32; ch = 32; bombs_number = 10;
            break;
        case Classic_Grid::MEDIUM:
            nw = 16; nh = 16; cw = 32; ch = 32; bombs_number = 40;
            break;
        case Classic_Grid::LARGE:
            nw = 30; nh = 16; cw = 32; ch = 32; bombs_number = 99;
            break;

        case Classic_Grid::DEBUG:
            nw = 3; nh = 3; cw = 300; ch = 300; bombs_number = 1;
            break;               
    }
    v2 pos =  Center_Pos(Grid_Size(nw, nh, cw, ch), game_state->screen_size);
    Grid_Update(game_state->current_grid, game_state->current_grid->index,
                nw, nh, cw, ch, pos.x, pos.y, bombs_number);
    
    Clear_Game_State(game_state);
    game_state->bomb_count = game_state->current_grid->bombs_number;
    Generate_Field(game_state->current_grid, false, 0);
}

// -----------------------------------------------------------------------------------------
#if 0 // @NOTE: Given up on mined out mode.
internal GAME_FUNCTION(New_Game_Mined)
{
    game_state->running_game = MINED;
    
    if(game_state->grid_count == 0) Init_Grids(game_state);
    if(game_state->grid_count > 1)
    {
        Keep_Main_Grid_Only(&game_state->grid);
        game_state->current_grid = game_state->grid;
        game_state->grid_count = 1;
    }

    int nw = 24, nh = 18, cw = 32, ch = 32;
    v2 pos =  Center_Pos(Grid_Size(nw, nh, cw, ch), game_state->screen_size);
    Grid_Update(game_state->current_grid, game_state->current_grid->index, 24, 18, 32, 32, pos.x, pos.y, 75);    
    
    Clear_Game_State(game_state);
    game_state->bomb_count = game_state->current_grid->bombs_number;    
    Generate_Field_Open_Path(game_state);
}
#endif

// -----------------------------------------------------------------------------------------
internal GAME_FUNCTION(New_Game_Parkour)
{
    game_state->running_game = PARKOUR;

    if(game_state->parkour_load_new_level)
    {
        if(game_state->grid) Keep_Main_Grid_Only(&game_state->grid);

        int level_to_load = game_state->parkour_data.load_level;
        int number_of_grid_in_level = game_state->parkour_data.array_of_levels[level_to_load].grid_count;
        game_state->grid_count = Add_nth_Grid(&game_state->grid, number_of_grid_in_level);

        // pointer for conveniance.
        Grid_Data* level = game_state->parkour_data.array_of_levels[level_to_load].level;
        
        Grid* current = game_state->grid;
        for(int i = 0; i < number_of_grid_in_level; ++i)
        {            
            Grid_Update(current,
                        level[i].index, 
                        level[i].nw, level[i].nh,
                        level[i].cw, level[i].ch,
                        level[i].pos_x, level[i].pos_y,
                        level[i].bombs_number);
            current = current->next;
        }

        game_state->parkour_load_new_level = false;
    }
    
    game_state->current_grid = game_state->grid;    
    Clear_Game_State(game_state);
    game_state->bomb_count = game_state->current_grid->bombs_number;
    
    Grid* generate_current = game_state->grid;
    while(generate_current)
    {
        Generate_Field(generate_current, false, 0);
        generate_current = generate_current->next;
    }
}

// -----------------------------------------------------------------------------------------
internal GAME_FUNCTION(New_Game)
{
    switch(game_state->mode)
    {
        case CLASSIC: { New_Game_Classic(game_state,0,0,0); } break;
#if 0
        case MINED:   { New_Game_Mined(game_state,0,0,0);   } break;
#endif
        case PARKOUR: { New_Game_Parkour(game_state,0,0,0); } break;
    }
}

// -----------------------------------------------------------------------------------------
inline GAME_FUNCTION(Menu_Up) { game_state->mode = MENU; }
inline GAME_FUNCTION(Quit) { game_state->running = false; }

// -----------------------------------------------------------------------------------------
internal GAME_FUNCTION(Small)
{
    if(game_state->classic_grid_size != Classic_Grid::SMALL)
    {
        game_state->classic_grid_size = Classic_Grid::SMALL;
        New_Game_Classic(game_state,0,0,0);   
    }
}
internal GAME_FUNCTION(Medium)
{
    if(game_state->classic_grid_size != Classic_Grid::MEDIUM)
    {
        game_state->classic_grid_size = Classic_Grid::MEDIUM;
        New_Game_Classic(game_state,0,0,0);   
    }
}
internal GAME_FUNCTION(Large)
{
    if(game_state->classic_grid_size != Classic_Grid::LARGE)
    {
        game_state->classic_grid_size = Classic_Grid::LARGE;
        New_Game_Classic(game_state,0,0,0);
    }
}

// -----------------------------------------------------------------------------------------
internal GAME_FUNCTION(Select_Classic)
{
    game_state->mode = CLASSIC;
    // @TODO: Will always resize to small, need to keep current size on victory and new game.    
    if(game_state->running_game != CLASSIC || game_state->game_win)
    {
        game_state->classic_grid_size = Classic_Grid::SMALL;
        New_Game_Classic(game_state,0,0,0);
    }
    else game_state->grid_need_update = true;
}
#if 0 // @NOTE: Given up on mined out mode.
internal GAME_FUNCTION(Select_Mined)
{
    game_state->mode = MINED;
    if(game_state->running_game != MINED)
    {
        New_Game_Mined(game_state,0,0,0);
    }
    else game_state->grid_need_update = true;
}
#endif
internal GAME_FUNCTION(Select_Parkour)
{
    game_state->mode = SELECT_PARKOUR_LEVEL;
    game_state->grid_need_update = true;
}

// -----------------------------------------------------------------------------------------
internal SELECT_LEVEL(Select_Level)
{
    //printf("Level %d\n", level);
    game_state->mode = PARKOUR;
    if(game_state->running_game != PARKOUR)
    {
        game_state->parkour_load_new_level = true;
        game_state->parkour_data.load_level = level;
        New_Game_Parkour(game_state,0,0,0);
    }
    else
    {
        if(game_state->parkour_data.load_level == level)
            game_state->grid_need_update = true;
        else
        {
            game_state->parkour_load_new_level = true;
            game_state->parkour_data.load_level = level;
            New_Game_Parkour(game_state,0,0,0);
        }
    }    
}


// -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------
internal void Generate_Bombs(Grid *grid, Cell *cells, int bombs_number)
{
    int n = 0;
    bool loop = true;
    while(loop)
    {
        int i = Get_Random_Number(0, grid->n_cells.w*grid->n_cells.h - 1);
        if(!(cells[i].flag & MASK_BOMB))
        {
            cells[i].flag |= MASK_BOMB;
            ++n;
        }

        if(n == bombs_number)
            loop = false;
    }    
}

// -----------------------------------------------------------------------------------------
internal void Generate_Bombs_Except(int this_cell, Grid *grid, Cell *cells, int bombs_number)
{
    int n = 0;
    bool loop = true;
    while(loop)
    {
        int i = Get_Random_Number(0, grid->n_cells.w*grid->n_cells.h - 1);
        if(!(cells[i].flag & MASK_BOMB) && i != this_cell)
        {
            cells[i].flag |= MASK_BOMB;
            ++n;
        }

        if(n == bombs_number)
            loop = false;
    }    
}

// -----------------------------------------------------------------------------------------
#if 0// @NOTE: Given up on mined out mode.
// @TODO: Can the end path be blocked by surending bombs?
internal void Generate_Bombs_Open_Path(Grid *grid, Cell *cells, int bombs_number)
{
    int n = 0;
    bool loop = true;
    while(loop)
    {
        int i = Get_Random_Number(0, grid->n_cells.w*grid->n_cells.h - 1);
        if(!(cells[i].flag & MASK_BOMB))
        {
            int x = (int)i % grid->n_cells.w;
            int y = (int)i / grid->n_cells.w;

            if( ( ((x < grid->start.x - 2) || (x > grid->start.x + 2)) ||
                  ((y < grid->start.y - 2) || (y > grid->start.y + 2)) ) &&
                ( ((x < grid->end.x - 2) || (x > grid->end.x + 2)) ||
                  ((y < grid->end.y - 2) || (y > grid->end.y + 2)) ) )
            {
                cells[i].flag |= MASK_BOMB;
                ++n;   
            }
        }

        if(n == bombs_number)
            loop = false;
    }    
}
#endif

// -----------------------------------------------------------------------------------------
internal void Generate_Field(Grid *current_grid, bool second_chance, int already_cleared)
{
    if(second_chance)
        Generate_Bombs_Except(already_cleared, current_grid, current_grid->cells, current_grid->bombs_number);
    else
        Generate_Bombs(current_grid, current_grid->cells, current_grid->bombs_number);

    int nw = current_grid->n_cells.w;
    int nh = current_grid->n_cells.h;

    for(int y = 0; y < nh; ++y)
    {
        for(int x = 0; x < nw; ++x)
        {
            int i = y * nw + x;
            if(!second_chance)
                current_grid->cells[i].flag |= MASK_COVER;
            else
                if(i != already_cleared) current_grid->cells[i].flag |= MASK_COVER;
                

            if(current_grid->cells[i].flag & MASK_BOMB)
                continue;
            
            u8 neighbours_bomb = Neighbours_Check(Is, MASK_BOMB, current_grid->cells, x, y, nw, nh);
                            
            if(neighbours_bomb > 0)
                current_grid->cells[i].value = neighbours_bomb;
        }
    }    
}

#if 0 // @NOTE: Given up on mined out mode.
// -----------------------------------------------------------------------------------------
internal void Generate_Field_Open_Path(Game_State *game_state)
{
    int nw = game_state->current_grid->n_cells.w;
    int nh = game_state->current_grid->n_cells.h;
    
    Generate_Bombs_Open_Path(game_state->current_grid, game_state->current_grid->cells, game_state->current_grid->bombs_number);
    game_state->current_grid->cells[game_state->current_grid->start.y * nw + game_state->current_grid->start.x].flag |= MASK_TESTED;

    for(int y = 0; y < nh; ++y)
    {
        for(int x = 0; x < nw; ++x)
        {
            int i = y * nw + x;
            if( ( ((x < game_state->current_grid->start.x) || (x > game_state->current_grid->start.x)) || 
                  ((y < game_state->current_grid->start.y) || (y > game_state->current_grid->start.y)) ) &&
                ( ((x < game_state->current_grid->end.x - 2) || (x > game_state->current_grid->end.x + 2)) ||
                  ((y < game_state->current_grid->end.y - 2) || (y > game_state->current_grid->end.y + 2)) ) )                
            {                
                    game_state->current_grid->cells[i].flag |= MASK_COVER;       
            }

            if(game_state->current_grid->cells[i].flag & MASK_BOMB)
                continue;

            u8 neighbours_bomb = Neighbours_Check(Is, MASK_BOMB, game_state->current_grid->cells, x, y, nw, nh);
                
            if(neighbours_bomb > 0)
                game_state->current_grid->cells[i].value = neighbours_bomb;
        }
    }    
}
#endif

// -----------------------------------------------------------------------------------------
internal void Check_Win(Game_State *game_state)
{
    int flaged_bomb = 0;
    int cover_left = 0;
    int false_flag = 0;
    for(int y = 0; y < game_state->current_grid->n_cells.y; ++y)
    {
        for(int x = 0; x < game_state->current_grid->n_cells.x; ++x)
        {
            int i = y * game_state->current_grid->n_cells.x + x;
            if((game_state->current_grid->cells[i].flag & MASK_BOMB) && (game_state->current_grid->cells[i].flag & MASK_FLAG))
                flaged_bomb++;
            if(!(game_state->current_grid->cells[i].flag & MASK_BOMB) && (game_state->current_grid->cells[i].flag & MASK_FLAG))
                false_flag++;
            if((game_state->current_grid->cells[i].flag & MASK_COVER) && !(game_state->current_grid->cells[i].flag & MASK_FLAG))
                cover_left++;
        }
    }

    if(flaged_bomb == game_state->current_grid->bombs_number)
        game_state->game_win = true;
    if((game_state->current_grid->bombs_number - flaged_bomb == 1) && (cover_left == 1) && (false_flag == 0))
        game_state->game_win = true;
}

// -----------------------------------------------------------------------------------------
internal void Pop_Up_Game_Classic_Win(Assets *assets, Game_State *game_state)
{
    Draw_Full_Rect_At(assets->grid_buffer->pixels, game_state->screen_size.w/4 , game_state->screen_size.h/4,
                      game_state->screen_size.w/2, game_state->screen_size.h/2,
                      assets->grid_buffer->pitch, assets->grid_buffer->bytes_per_pixels,
                      game_colors[colors::COVER]);

    switch(game_state->classic_grid_size)
    {
        case Classic_Grid::SMALL:
        {
            if(!game_state->recorded_times[Classic_Grid::SMALL] || (game_state->timer < game_state->recorded_times[Classic_Grid::SMALL]))
            {
                game_state->recorded_times[Classic_Grid::SMALL] = game_state->timer;
                Save_Best_Time(game_state);
            }
        } break;
        case Classic_Grid::MEDIUM:
        {
            if(!game_state->recorded_times[Classic_Grid::MEDIUM] || (game_state->timer < game_state->recorded_times[Classic_Grid::MEDIUM]))
            {
                game_state->recorded_times[Classic_Grid::MEDIUM] = game_state->timer;
                Save_Best_Time(game_state);
            }            
        } break;
        case Classic_Grid::LARGE:
        {
            if(!game_state->recorded_times[Classic_Grid::LARGE] || (game_state->timer < game_state->recorded_times[Classic_Grid::LARGE]))
            {
                game_state->recorded_times[Classic_Grid::LARGE] = game_state->timer;
                Save_Best_Time(game_state);
            }
        } break;            
    }

    char record[64];
    int w, h;
    v2 starting_pos = { game_state->screen_size.w/4 + 100, game_state->screen_size.h/4 + 50 };
    
    sprintf(record, "Best time :");
    Get_String_Size(record, 16, &w, &h);
    Make_String(assets->grid_buffer, assets->font_bitmap, record, 16,
                starting_pos.x, starting_pos.y, w, h);
        
    if(game_state->recorded_times[Classic_Grid::SMALL])
    {
        //sprintf(record, "    - Time: %us , beginner difficulty", game_state->recorded_times[Classic_Grid::SMALL]);
        sprintf(record, "    - Difficulty beginner : %us", game_state->recorded_times[Classic_Grid::SMALL]);
        starting_pos.y += h;
        Get_String_Size(record, 16, &w, &h);
        Make_String(assets->grid_buffer, assets->font_bitmap, record, 16,
                    starting_pos.x, starting_pos.y, w, h);
    }

    if(game_state->recorded_times[Classic_Grid::MEDIUM])
    {
        sprintf(record, "    - Difficulty   medium : %us", game_state->recorded_times[Classic_Grid::MEDIUM]);
        starting_pos.y += h;
        Get_String_Size(record, 16, &w, &h);
        Make_String(assets->grid_buffer, assets->font_bitmap, record, 16,
                    starting_pos.x, starting_pos.y, w, h);
    }

    if(game_state->recorded_times[Classic_Grid::LARGE])
    {
        sprintf(record, "    - Difficulty    large : %us", game_state->recorded_times[Classic_Grid::LARGE]);
        starting_pos.y += h;
        Get_String_Size(record, 16, &w, &h);
        Make_String(assets->grid_buffer, assets->font_bitmap, record, 16,
                    starting_pos.x, starting_pos.y, w, h);
    }
}

// -----------------------------------------------------------------------------------------
internal void Render_Game(Rendering_Context *rendering_context, Assets *assets, Grid *grid, UI_Context *ui_ctx)
{    
    Bit_Blt(assets->grid_buffer, NULL, rendering_context->back_buffer, NULL);
    Bit_Blt(assets->ui_layer, NULL, rendering_context->back_buffer, NULL);
}

// -----------------------------------------------------------------------------------------
internal void Update_And_Render(Rendering_Context *rendering_ctx, Assets *assets, Game_State *game_state, UI_Context *ui_ctx)
{
    UI_Update(game_state, rendering_ctx, ui_ctx, assets);
    DEBUG_drag_grid(game_state, ui_ctx, assets);
    
    switch(game_state->mode)
    {
        case MENU:
        {
            //MEMSET_BUFFER_TO_GRADIENT(assets->grid_buffer->pixels, assets->grid_buffer->w, assets->grid_buffer->h, assets->grid_buffer->pitch, game_colors[colors::BACKGROUND]);
            Bit_Blt(assets->menu_background, NULL, assets->grid_buffer, NULL);
        } break;

        case SELECT_PARKOUR_LEVEL:
        {
            MEMSET_BUFFER_TO_GRADIENT(assets->grid_buffer->pixels, assets->grid_buffer->w, assets->grid_buffer->h, assets->grid_buffer->pitch, game_colors[colors::BACKGROUND]);
        } break;
            
        case CLASSIC:
#if 0
        case MINED:
#endif
        case PARKOUR:
        {
            if(game_state->grid_need_update || game_state->init) 
            {
                MEMSET_BUFFER_TO_GRADIENT(assets->grid_buffer->pixels, assets->grid_buffer->w, assets->grid_buffer->h, assets->grid_buffer->pitch, game_colors[colors::BACKGROUND]);

                // Grid Highlight
                // @TODO: Something better then drawing full rectangle behind?
                Draw_Full_Rect_At(assets->grid_buffer->pixels, game_state->current_grid->pos.x - 5, game_state->current_grid->pos.y - 5,
                                  game_state->current_grid->size.w + 10, game_state->current_grid->size.h + 10,
                                  assets->grid_buffer->pitch, assets->grid_buffer->bytes_per_pixels,
                                  game_colors[colors::INTERACTING]);
                
                Grid* current = game_state->grid;
                while(current)
                {
                    Update_Grid_Buffer(assets, game_state, current);
#if 0
                    if(current->next)
                    {
                        Draw_Line(assets->grid_buffer->pixels, assets->grid_buffer->pitch, assets->grid_buffer->bytes_per_pixels,
                                  current->pos.x, current->pos.y, current->next->pos.x, current->next->pos.y,
                                  game_colors[colors::LIGNE]);
                    }
#endif
                    current = current->next;
                }        
                game_state->init = false;       
            }                
        } break;

        case CLASSIC_WIN:
        {
            //MEMSET_BUFFER_TO_GRADIENT(assets->grid_buffer->pixels, assets->grid_buffer->w, assets->grid_buffer->h, assets->grid_buffer->pitch, game_colors[colors::BACKGROUND]);
            Pop_Up_Game_Classic_Win(assets, game_state);
        } break;
    }

    Render_Game(rendering_ctx, assets, game_state->current_grid, ui_ctx);
}

// -----------------------------------------------------------------------------------------
internal void Game_Update_And_Render(Rendering_Context *rendering_ctx, Game_State *game_state, Assets *assets, UI_Context* ui_ctx)    
{
    game_state->grid_need_update = false;
    
    if(!game_state->game_over && !game_state->game_win && game_state->current_grid && game_state->mode != MENU)
    {
        int nw = game_state->current_grid->n_cells.w;
        int nh = game_state->current_grid->n_cells.h;

        if(game_state->propagate)
        {
            game_state->grid_need_update = true;        
            
            size_t current_length = game_state->cells_to_test.length;
            for(size_t i = 0; i < current_length; ++i)
            {
                int index = game_state->cells_to_test.element[i];
                if(!game_state->current_grid->cells[index].value && !(game_state->current_grid->cells[index].flag & MASK_BOMB))
                {
                    int x = (int)index % nw;
                    int y = (int)index / nw;
                    Push_Cell_to_Test(game_state, x, y, nw, nh);
                }            
            }

            // Pop n cell per frame. This could change base on mode but,
            // @IMPORTANT: CANT be superior to the actual length of the cell to test!!!
            size_t number_of_pop_per_frame = current_length;
            for(size_t p = 0; p < number_of_pop_per_frame; ++p)
            {
                if(game_state->current_grid->cells[game_state->cells_to_test.element[0]].flag & MASK_COVER)
                {
                    if(!(game_state->current_grid->cells[game_state->cells_to_test.element[0]].flag & MASK_FLAG))
                        game_state->current_grid->cells[game_state->cells_to_test.element[0]].flag ^= MASK_COVER;
                }
                pop(&game_state->cells_to_test);   
            }

            if(game_state->cells_to_test.length == 0)
                game_state->propagate = false;
        }

    
        if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y,
                         game_state->current_grid->pos.x,
                         game_state->current_grid->pos.y,
                         game_state->current_grid->size.w, game_state->current_grid->size.h))
        {
            // subtract the offset to get proper index
            int cell_x = (int)((ui_ctx->mouse.x - (game_state->current_grid->pos.x)) / game_state->current_grid->cell.w);
            int cell_y = (int)((ui_ctx->mouse.y - (game_state->current_grid->pos.y)) / game_state->current_grid->cell.h);
        
            int i = cell_y * nw + cell_x;   
    
            if(ui_ctx->mouse.left_button.was_down && ui_ctx->mouse.left_button.release)
            {
                game_state->grid_need_update = true;
                
                if(game_state->click_counter < 1) 
                {
                    ++game_state->click_counter;
                    game_state->first_click_check = true;      
                }
                else game_state->first_click_check = false;
                
                bool testing_condition_based_on_mode;
                switch(game_state->mode)
                {
                    case CLASSIC:
                    {
                        testing_condition_based_on_mode = (game_state->current_grid->cells[i].flag & MASK_COVER) && !(game_state->current_grid->cells[i].flag & MASK_BOMB) &&
                            !(game_state->current_grid->cells[i].flag & (MASK_FLAG | MASK_QUESTION_MARK));
                    } break;

#if 0 // @NOTE: Given up on mined out mode.
                    case MINED:
                    {
                        u8 neighbouring_empty_cells = Neighbours_Check(IsNot, MASK_COVER, game_state->current_grid->cells, cell_x, cell_y, nw, nh);
                    
                        testing_condition_based_on_mode = (game_state->current_grid->cells[i].flag & MASK_COVER) && !(game_state->current_grid->cells[i].flag & MASK_BOMB) &&
                            neighbouring_empty_cells && !(game_state->current_grid->cells[i].flag & (MASK_FLAG | MASK_QUESTION_MARK));               
                    } break;
#endif

                    case PARKOUR:
                    {
                        testing_condition_based_on_mode = (game_state->current_grid->cells[i].flag & MASK_COVER) && !(game_state->current_grid->cells[i].flag & MASK_BOMB) &&
                            !(game_state->current_grid->cells[i].flag & (MASK_FLAG | MASK_QUESTION_MARK));
                    } break;
                }

                if(testing_condition_based_on_mode)
                {
                    if(!game_state->last_time) game_state->last_time = SDL_GetTicks();
                
                    game_state->current_grid->cells[i].flag = 0;
                    if(!game_state->current_grid->cells[i].value)   // empty cell
                    {                    
                        game_state->propagate = true;
                        Push_Cell_to_Test(game_state, cell_x, cell_y, nw, nh);
                    }   
                }
        
                if((game_state->current_grid->cells[i].flag & MASK_COVER) && (game_state->current_grid->cells[i].flag & MASK_BOMB) && 
                   !(game_state->current_grid->cells[i].flag & (MASK_FLAG | MASK_QUESTION_MARK)))
                {
                    // Generate new field with clear cover and no bomb there.
                    if(game_state->first_click_check)
                    {
                        memset(game_state->current_grid->cells, 0, sizeof(Cell) * game_state->current_grid->n_cells.h * game_state->current_grid->n_cells.w);
                        Generate_Field(game_state->current_grid, true, i);
                        if(!game_state->current_grid->cells[i].value)
                        {
                            game_state->propagate = true;
                            Push_Cell_to_Test(game_state, cell_x, cell_y, nw, nh);
                        }                        
                    }
                    else
                    {
                        game_state->game_over = true;
                        game_state->current_grid->cells[i].flag |= MASK_EXPLODE;
                        //printf("Game Over\n");   
                    }
                }
            }

            if(ui_ctx->mouse.right_button.was_down && ui_ctx->mouse.right_button.release)
            {
                game_state->grid_need_update = true;
                
                if(game_state->current_grid->cells[i].flag & MASK_COVER)
                {
                    if(game_state->current_grid->cells[i].flag & MASK_FLAG)
                    {
                        game_state->current_grid->cells[i].flag ^= MASK_FLAG;
                        game_state->current_grid->cells[i].flag ^= MASK_QUESTION_MARK;
                        if(game_state->bomb_count < game_state->current_grid->bombs_number) game_state->bomb_count++;
                    }
                    else if(game_state->current_grid->cells[i].flag & MASK_QUESTION_MARK)
                    {
                        game_state->current_grid->cells[i].flag ^= MASK_QUESTION_MARK;
                    }
                    else
                    {
                        if(game_state->bomb_count > 0)
                        {
                            game_state->bomb_count--;
                            game_state->current_grid->cells[i].flag ^= MASK_FLAG;                        
                        }
                    }
                }
            }
        }

        // -------------------------------------------------------------
        if((game_state->mode != MENU) && game_state->last_time)
        {
            u32 new_time = SDL_GetTicks();
            if(new_time >= game_state->last_time + 1000)
            {
                game_state->timer++;
                game_state->last_time = new_time;
            }
        }
    }
    
    // -------------------------------------------------------------

    if(game_state->current_grid)
        if((game_state->bomb_count == 0) || (game_state->bomb_count == 1))
            Check_Win(game_state);

    
    // @NOTE: If grid_need_update isn't set here during a game_over the grid buffer is never updated,
    // and the flag or question mark will still be drawned under the bomb.
    if(game_state->game_over) game_state->grid_need_update = true;

    if(game_state->game_win)
    {
        switch(game_state->mode)
        {
            case CLASSIC:
            {
                game_state->mode = CLASSIC_WIN;                
            } break;
            
#if 0
            case MINED: {} break;
#endif
            
            case PARKOUR:
            {
                if(!game_state->parkour_finished)
                {
                    if(game_state->current_grid->next)
                    {
                        game_state->click_counter = 0;
                        game_state->current_grid = game_state->current_grid->next;
                        game_state->bomb_count = game_state->current_grid->bombs_number;
                        game_state->game_win = false;
                    }
                    else 
                    {
                        if(game_state->parkour_data.load_level == game_state->parkour_data.number_of_levels)
                        {
                            game_state->parkour_finished = true;   
                        }
                        else
                        {
                            game_state->parkour_load_new_level = true;
                            game_state->parkour_data.load_level++;
                            New_Game_Parkour(game_state, 0,0,0);
                        }
                    }                    
                }
            } break;
        }
    }   
    
    Update_And_Render(rendering_ctx, assets, game_state, ui_ctx);

    ui_ctx->mouse.left_button.was_down = ui_ctx->mouse.left_button.is_down;
    ui_ctx->mouse.right_button.was_down = ui_ctx->mouse.right_button.is_down;
}





//##########################################################################################
//################################## No Longer Supported ###################################
#if 0
// @TODO: Probably NEVER gonna use this but kept around just in case in its own file for conveniance.
#include "other_field_test.cpp"
#endif

#if 0
// -----------------------------------------------------------------------------------------
internal void Resize_Buffers(Assets *assets, Game_State *game_state, UI_Context *ui_ctx)
{
    // @TODO: Dont reallocate! Only need to change width/height/pitch.
    assets->grid_buffer = Resize_Surface(&assets->grid_buffer, game_state->screen_size.w, game_state->screen_size.h);
    assets->temp_string = Resize_Surface(&assets->temp_string, game_state->screen_size.w, game_state->screen_size.h);
    assets->ui_layer    = Resize_Surface(&assets->ui_layer, game_state->screen_size.w, game_state->screen_size.h);
}

// -----------------------------------------------------------------------------------------
internal void Resize_UI(UI_Context *ui_ctx, v2 size)
{
    for(int i = 0; i < MAX_UI_ELEMENT; ++i)
        ui_ctx->ui_elements[i].rect = ui_ctx->ui_elements[i].get_rect_fn(size);
}

// -----------------------------------------------------------------------------------------
internal void Resize_Game(Rendering_Context *rendering_ctx,
                          Game_State *game_state, Assets *assets, UI_Context *ui_ctx)
{
    Resize_Buffers(assets, game_state, ui_ctx);
    Resize_UI(ui_ctx, game_state->screen_size);
    SDL_Resize_Window(rendering_ctx, game_state->screen_size.w, game_state->screen_size.h);
}

// -----------------------------------------------------------------------------------------
inline void Update_Screen_Size(Game_State *game_state, Game_Screen::Size size)
{
    game_state->game_screen_size = size;
    switch(size)
    {
        case Game_Screen::DEFAULT:
        {
            game_state->screen_size.w = 1280;
            game_state->screen_size.h = 720;
        } break;

        case Game_Screen::BIG:
        {
            game_state->screen_size.w = 1500;
            game_state->screen_size.h = 940;            
        } break;
    }    
}

inline void Update_Screen_Size(v2 *screen_size, int w, int h)
{
    screen_size->w = w;
    screen_size->h = h;
}
#endif
//##########################################################################################
//##########################################################################################
