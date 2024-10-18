
// @NOTE: HEAD MUST BE SET TO NULL
// -----------------------------------------------------------------------------------------
internal u8 Add_Grid(Grid **grid_head)
{
    Grid* new_grid = (Grid *)malloc(sizeof(Grid));
    new_grid->index = 0;
    new_grid->cells = NULL;
    new_grid->next = NULL;
    u8 count = 1;    // for the new grid
    
    if(!*grid_head) *grid_head = new_grid;
    else
    {
        Grid* current = *grid_head;    
        while(current->next)
        {
            current = current->next;
            ++count;
        }
        new_grid->index = count++;
        current->next = new_grid;   
    }
    
    return count;
}

// -----------------------------------------------------------------------------------------
internal bool Delete_Grid(Grid **grid_head, u8 index)
{
    bool success = false;
    
    Grid* current = *grid_head;
    if(current->index == index)
    {
        Grid* next = current->next;
        free(current);
        *grid_head = next;
        success = true;
    }
    else
    {
        while(current->next)
        {
            if(current->next->index == index)
            {
                Grid* next = current->next->next;
                free(current->next);
                current->next = next;
                success = true;
                break;
            }
            current = current->next;
        }
    }

    return success;
}

// -----------------------------------------------------------------------------------------
internal u8 Add_nth_Grid(Grid **grid_head, int n)
{
    assert(n > 0);
    u8 count = 0;
    if(!*grid_head) count = Add_Grid(grid_head);
    
    for(int i = 0; i < n-1; ++i) count = Add_Grid(grid_head);

    return count;
}

// -----------------------------------------------------------------------------------------
internal void Keep_Main_Grid_Only(Grid **grid_head)
{
    Grid* current = *grid_head;    
    Grid* next = current->next;
    // "Cut" the link with the head and start deleting at that point.
    current->next = NULL;
    current = next;
    
    while(current)
    {
        next = current->next;
        free(current->cells);
        free(current);
        current = next;
    }    
}

// @TODO: Why Init? when do you need it?
// -----------------------------------------------------------------------------------------
internal void Init_Grids(Game_State *game_state)
{
    game_state->grid = NULL;
    game_state->grid_count = Add_Grid(&game_state->grid);
    game_state->current_grid = game_state->grid;    
}


// -----------------------------------------------------------------------------------------
internal void Generate_Cells(Grid *grid)
{
    if(grid->cells)
        grid->cells = (Cell *)realloc(grid->cells, sizeof(Cell) * grid->n_cells.w * grid->n_cells.h);
    else
        grid->cells = (Cell *)malloc(sizeof(Cell) * grid->n_cells.w * grid->n_cells.h);
    memset(grid->cells, 0, sizeof(Cell) * grid->n_cells.w * grid->n_cells.h);
}

// -----------------------------------------------------------------------------------------
internal void Grid_Update(Grid *grid,int index, int nw, int nh, int cw, int ch, int pos_x, int pos_y, int bombs_number)
{
    grid->index        = index;
    
    grid->cell         = { cw, ch };    
    grid->n_cells      = { nw, nh };
    grid->size         = { nw * cw, nh * ch };
    grid->pos          = { pos_x, pos_y };   
    grid->bombs_number = bombs_number;

    Generate_Cells(grid);
    
    // @TODO: THIS IS FOR TESTING.
    grid->start = { 0, 0 };
    grid->end = { grid->n_cells.w-1, grid->n_cells.h-1 };    
}

// -----------------------------------------------------------------------------------------
internal void Update_Grid_Buffer(Assets *assets, Game_State *game_state, Grid *current_grid)
{        
    int nw = current_grid->n_cells.w;
    int nh = current_grid->n_cells.h;
    int cw = current_grid->cell.w;
    int ch = current_grid->cell.h;    


    for(int y = 0; y < nh; ++y)
    {
        for(int x = 0; x < nw; ++x)
        {
            int i = y * nw + x;            
            int px = (x * cw) + current_grid->pos.x;
            int py = (y * ch) + current_grid->pos.y;

            if(px < 0 || py < 0)
                continue;

            // @TODO: Expensive to calculate color every square?
            // ---------------------------------------------------------------------
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
            Draw_Full_Rect_At(assets->grid_buffer->pixels,
                              px, py, cw, ch,
                              assets->grid_buffer->pitch, assets->grid_buffer->bytes_per_pixels,
                              grid_color);                    
            // ---------------------------------------------------------------------
            if(current_grid->cells[i].flag & MASK_COVER)
            {
                SDL_Rect rect = { px, py, cw, ch };
                Bit_Blt(assets->cover, NULL, assets->grid_buffer, &rect);
            }
            else
            {
                u8 set = current_grid->cells[i].value;
                if(!set) continue;

                int new_w = (assets->font_asset[set]->w * cw) / (32);
                int new_h = (assets->font_asset[set]->h * ch) / (32);
                SDL_Rect rect = {
                    px + ((cw / 2) - (new_w / 2)),
                    py + ((ch / 2) - (new_h / 2)),
                    new_w, new_h
                };

                Bit_Blt(assets->font_asset[set], NULL, assets->grid_buffer, &rect);
            }

            if(current_grid->cells[i].flag & MASK_FLAG) 
            {
                SDL_Rect rect = { px, py, cw, ch };
                Bit_Blt(assets->flag, NULL, assets->grid_buffer, &rect);
            }

            if(current_grid->cells[i].flag & MASK_QUESTION_MARK)
            {
                int new_w = (assets->font_asset[QUESTION_MARK]->w * cw) / (32);
                int new_h = (assets->font_asset[QUESTION_MARK]->h * ch) / (32);
                SDL_Rect rect = {
                    px + ((cw / 2) - (new_w / 2)),
                    py + ((ch / 2) - (new_h / 2)),
                    new_w, new_h
                };
                Bit_Blt(assets->font_asset[QUESTION_MARK], NULL, assets->grid_buffer, &rect);
            }

            if(game_state->game_over && (current_grid->cells[i].flag & MASK_BOMB))
            {
                if(current_grid->cells[i].flag & MASK_FLAG) current_grid->cells[i].flag ^= MASK_FLAG;
                if(current_grid->cells[i].flag & MASK_QUESTION_MARK) current_grid->cells[i].flag ^= MASK_QUESTION_MARK;

                SDL_Rect rect = { px, py, cw, ch };
                if(current_grid->cells[i].flag & MASK_EXPLODE)
                    Bit_Blt(assets->bomb_explode, NULL, assets->grid_buffer, &rect);
                else
                    Bit_Blt(assets->bomb, NULL, assets->grid_buffer, &rect);
            }
        }
    }
}
