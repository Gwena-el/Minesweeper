#if !defined(UI_MINESWEEPER_CONTEXT)


#define GET_UI_RECT(name) SDL_Rect name(v2 size)
typedef GET_UI_RECT(get_ui_rect);



struct Button
{
    bool was_down;
    bool is_down;

    bool release;
};

struct Mouse
{
    int x, y;
    Button left_button, right_button;

};


enum UI_Type
{
    BUTTON,
    LEVEL_BUTTON,
    TIMER,
    BOMB_COUNTER,

    MAX_UI_TYPE,
};

struct UI_Element
{
    UI_Type type;
    int level_to_select;
    
    game_function* fn;    
    select_level* select_lvl;    

    SDL_Surface** btn_bmp;
    SDL_Rect rect;
    get_ui_rect* get_rect_fn;
};

#if 0
enum UI_Element_List
{
    UI_NEW_GAME,
    UI_MENU,
    
    UI_TIMER,
    UI_BOMB_COUNTER,

    UI_RESIZE_SMALL,
    UI_RESIZE_MEDIUM,
    UI_RESIZE_LARGE,

    UI_SELECT_CLASSIC,
    UI_SELECT_MINED,
    UI_SELECT_PARKOUR,
    UI_BACK_SELECT_PARKOUR,

    MAX_UI_ELEMENT,
};
#endif

typedef UI_Element* array_of_ui_element;
struct UI_Context
{
    Mouse       mouse;
    
    //array_of_ui_element ui_elements;

    u8 menu_ui_count;
    array_of_ui_element menu;    
    
    u8 classic_ui_count;
    array_of_ui_element classic;
    u8 classic_win_ui_count;
    array_of_ui_element classic_win;    
    
    u8 mined_ui_count;
    array_of_ui_element mined;
    
    u8 parkour_ui_count;
    array_of_ui_element parkour;

    
    u8 select_parkour_level_ui_count;
    array_of_ui_element select_parkour_level_ui;

    UI_Element* select_level_ui;
   

    UI_Element* hot;
    UI_Element* next_hot;
    UI_Element* interacting;
};



#define UI_MINESWEEPER_CONTEXT
#endif
