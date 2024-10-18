#if !defined(MINESWEEPER)

namespace colors
{
    enum {
        BACKGROUND,
        BACKGROUND_GRID,
        LIGNE,
        COVER,

        EXPLODE,

        // Button
        DEFAULT,
        HOT,
        INTERACTING,

        MAX_COLOR,
    };
}

union v2
{
    struct
    {
        int x;
        int y;  
    };
    struct
    {
        int w;
        int h;
    };
};

struct Cell
{
    u8 value;
    u8 flag;
};

enum Game_Mode
{
    NONE,
    
    MENU,
    SELECT_PARKOUR_LEVEL,
    
    CLASSIC,
    MINED,
    PARKOUR,
    TEST1,

    CLASSIC_WIN,

    MAX_MODE
};

namespace Classic_Grid
{
    enum Size {
        SMALL,
        MEDIUM,
        LARGE,

        DEBUG,

        MAX_SIZE,
    };   
}
#if 0
namespace Game_Screen
{
    enum Size {
        DEFAULT,
        BIG,
        CUSTOM,

        MAX_SCREEN_SIZE,
    };
}
#endif

// -----------------------------------------------------------------------------------------
struct Grid_Data
{
    int index, nw, nh, cw, ch, pos_x, pos_y, bombs_number;
};

struct Level_Array
{
    Grid_Data* level;
    int grid_count;
};

struct Parkour_Data
{
    Level_Array* array_of_levels;
    int number_of_levels;
    int load_level;
};

struct Grid
{
    u8 index;

    // @TODO: Replace to scale?
    v2 n_cells;
    v2 cell;
    v2 padding;
    v2 size;
    v2 pos;

    int bombs_number;
    Cell* cells;

    v2 start;
    v2 end;

    Grid* next;
};

struct Best_Time
{
    char difficulty[8];
    u32 recorded_time;
};

struct Game_State
{
    bool running;
    
    u32 last_time;
    u32 timer;
    //Best_Time best_time;
    // @TODO: Once rid of DEBUG Size use Classic_Grid::MAX_SIZE instead of 3.
    u32 recorded_times[3];
    //u32 recorded_time;

    bool first_click_check;
    u8 click_counter;
    bool game_over;
    bool game_win;
    bool parkour_finished;

    int bomb_count;

    u8 grid_count;          
    Grid* grid;               // link list of grids
    Grid* current_grid;

    bool propagate;
    // @TODO: Should cells_to_test be bound to its specifics grid.
    dynamic_array_int cells_to_test;

    Game_Mode running_game;
    Game_Mode mode;

    Classic_Grid::Size classic_grid_size;
    //Game_Screen::Size game_screen_size;
    v2 screen_size;
    v2 screen_padding;

    bool init;
    bool grid_need_update;    

    bool parkour_load_new_level;
    Parkour_Data parkour_data;

    // smth smth best time.

    //rgb_color colors[colors::MAX_COLOR];
};

enum Mask
{
    MASK_BOMB           =    0x01,
    MASK_FLAG           =    0x02,
    MASK_QUESTION_MARK  =    0x04,                            
    MASK_COVER          =    0x08,

    MASK_TESTED         =    0x10,
    MASK_EXPLODE        =    0x20,

    MAX_MASK,
};

/*

  0101     0101     0110 
& 0110   | 0110   ^ 0011 
------   ------   ------
  0100     0111     0101

*/

#define MEMSET_BUFFER_TO_COLOR(pixels, w, h, pitch, color)    Draw_Full_Rect(pixels, w, h, pitch, color)
#define MEMSET_BUFFER_TO_GRADIENT(pixels, w, h, pitch, color) Draw_Background_Gradient(pixels, w, h, pitch, color)


struct UI_Context;

// @NOTE: Parameters are far from ideal... pass null if doesn't need it.
#define GAME_FUNCTION(name) void name(Game_State *game_state, Rendering_Context *rendering_ctx, Assets *assets, UI_Context *ui_ctx)
typedef GAME_FUNCTION(game_function);

#define SELECT_LEVEL(name) void name(Game_State *game_state, int level)
typedef SELECT_LEVEL(select_level);


inline bool Point_In_Rect(int x, int y, int rx, int ry, int rw, int rh);
internal void Generate_Field(Grid *current_grid, bool second_chance, int already_cleared);
internal void Generate_Field_Open_Path(Game_State *game_state);
internal void Resize_Game(Rendering_Context *rendering_ctx, Game_State *game_state, Assets *assets, UI_Context *ui_ctx);

internal GAME_FUNCTION(New_Game_Classic);
internal GAME_FUNCTION(New_Game_Mined);
internal GAME_FUNCTION(New_Game_Parkour);
internal GAME_FUNCTION(New_Game);

inline GAME_FUNCTION(Menu_Up);
inline GAME_FUNCTION(Quit);

internal GAME_FUNCTION(Small);
internal GAME_FUNCTION(Medium);
internal GAME_FUNCTION(Large);

internal GAME_FUNCTION(Select_Classic);
#if 0 // @NOTE: Given up on mined out mode.
internal GAME_FUNCTION(Select_Mined);
#endif
internal GAME_FUNCTION(Select_Parkour);

internal SELECT_LEVEL(Select_Level);



#define MINESWEEPER
#endif
