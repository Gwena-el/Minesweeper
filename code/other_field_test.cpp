// -----------------------------------------------------------------------------------------
inline bool cell_is_free(Cell *cells, int x, int y, int nw, int nh)
{
    if( (x >= 0) && (x < nw) &&
        (y >= 0) && (y < nh) )
    {
        int i = y * nw + x;
        if(!(cells[i].value) && !(cells[i].flag))
           return 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------------------
inline int cell_value(Cell *cells, int x, int y, int nw, int nh)
{
    if( (x >= 0) && (x < nw) &&
        (y >= 0) && (y < nh) )
    {
        int i = y * nw + x;
        if(cells[i].value && !(cells[i].flag)) return cells[i].value;
    }

    return 0;
}


 // -----------------------------------------------------------------------------------------
internal void Generate_Other_Field(Game_State *game_state)
{
    int nw = game_state->grid.n_cells.w;
    int nh = game_state->grid.n_cells.h;

#if 1
    Generate_Bombs(&game_state->grid, game_state->cells, game_state->grid.bombs_number);
#else
    uint8_t test_map[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 1, 0, 0,
        1, 1, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 1, 0, 0, 0, 0,        
    };

    for(int i = 0; i < nw * nh; ++i)
    {
        if(test_map[i])
            game_state->cells[i].flag |= MASK_BOMB;
    }
#endif

    int bombs_index[game_state->grid.bombs_number] = {};
    int bomb_counter = 0;
    for(int y = 0; y < nh; ++y)
    {
        for(int x = 0; x < nw; ++x)
        {
            int i = y * nw + x;
            if(!(game_state->cells[i].flag & MASK_BOMB)) continue;

            game_state->cells[i].flag |= MASK_COVER;

            uint8_t neighbouring_empty_cells =
                IsNot(MASK_BOMB, game_state->cells, x-1, y-1, nw, nh) + IsNot(MASK_BOMB, game_state->cells, x  , y-1, nw, nh) +
                IsNot(MASK_BOMB, game_state->cells, x+1, y-1, nw, nh) + IsNot(MASK_BOMB, game_state->cells, x-1, y  , nw, nh) +
                IsNot(MASK_BOMB, game_state->cells, x+1, y  , nw, nh) + IsNot(MASK_BOMB, game_state->cells, x-1, y+1, nw, nh) +
                IsNot(MASK_BOMB, game_state->cells, x  , y+1, nw, nh) + IsNot(MASK_BOMB, game_state->cells, x+1, y+1, nw, nh);

            game_state->cells[i].bomb.free_cells = neighbouring_empty_cells;

            bombs_index[bomb_counter++] = i;
        }
    }

    //Sort_Bomb_Based_On_Free_Cells(game_state->bombs, game_state->bombs_number);
    for(int counter = 0; counter < game_state->grid.bombs_number - 1; ++counter)
    {
        for(int c = 0; c < game_state->grid.bombs_number - counter - 1; ++c)
        {
            if(game_state->cells[bombs_index[c]].bomb.free_cells > game_state->cells[bombs_index[c + 1]].bomb.free_cells)
            {
                // Swap
                int temp_index = bombs_index[c];
                bombs_index[c] = bombs_index[c + 1];               
                bombs_index[c + 1] = temp_index;
            }
        }
    }

    // @NOTE: to test things out, I will increment each value by 1.
    int increment_value = 1;
    int previous_value = 0;
    for(int b = 0; b < game_state->grid.bombs_number; ++b)
    {
        if( (b > 0) && (game_state->cells[bombs_index[b]].bomb.free_cells == game_state->cells[bombs_index[b-1]].bomb.free_cells) )
            game_state->cells[bombs_index[b]].bomb.value = previous_value;
        else
            game_state->cells[bombs_index[b]].bomb.value = previous_value + increment_value;
        
        previous_value = game_state->cells[bombs_index[b]].bomb.value;            
    }

}

// -----------------------------------------------------------------------------------------
internal void Process_Other_Field(Game_State *game_state)
{
    int nw = game_state->grid.n_cells.w;
    int nh = game_state->grid.n_cells.h;

    for(int y = 0; y < nh; ++y)
    {
        for(int x = 0; x < nw; ++x)
        {
            int i = y * nw + x;
            if((game_state->cells[i].flag & MASK_BOMB) || (game_state->cells[i].flag & MASK_TESTED)) continue;

            int bombs_to_update[8] = {};
            int bomb_counter = 0;
            if(Is(MASK_BOMB, game_state->cells, x-1, y-1, nw, nh))
                bombs_to_update[bomb_counter++] = (y-1) * nw + (x-1);
            if(Is(MASK_BOMB, game_state->cells, x  , y-1, nw, nh))
                bombs_to_update[bomb_counter++] = (y-1) * nw + x;
            if(Is(MASK_BOMB, game_state->cells, x+1, y-1, nw, nh))
                bombs_to_update[bomb_counter++] = (y-1) * nw + (x+1);
            if(Is(MASK_BOMB, game_state->cells, x-1, y  , nw, nh))
                bombs_to_update[bomb_counter++] = y * nw + (x-1);
            if(Is(MASK_BOMB, game_state->cells, x+1, y  , nw, nh))
                bombs_to_update[bomb_counter++] = y * nw + (x+1);
            if(Is(MASK_BOMB, game_state->cells, x-1, y+1, nw, nh))
                bombs_to_update[bomb_counter++] = (y+1) * nw + (x-1);
            if(Is(MASK_BOMB, game_state->cells, x  , y+1, nw, nh))
                bombs_to_update[bomb_counter++] = (y+1) * nw + x;
            if(Is(MASK_BOMB, game_state->cells, x+1, y+1, nw, nh))
                bombs_to_update[bomb_counter++] = (y+1) * nw + (x+1);

            if(bomb_counter == 0) continue;
            
            for(int b = 0; b < bomb_counter; ++b)
            {
                int index = bombs_to_update[b];
                int bomb_x = (int)index % nw;
                int bomb_y = (int)index / nw;
                
                int free_neighbours =
                    cell_is_free(game_state->cells, bomb_x-1, bomb_y-1, nw, nh) + cell_is_free(game_state->cells, bomb_x  , bomb_y-1, nw, nh) +
                    cell_is_free(game_state->cells, bomb_x+1, bomb_y-1, nw, nh) + cell_is_free(game_state->cells, bomb_x-1, bomb_y  , nw, nh) +
                    cell_is_free(game_state->cells, bomb_x+1, bomb_y  , nw, nh) + cell_is_free(game_state->cells, bomb_x-1, bomb_y+1, nw, nh) +
                    cell_is_free(game_state->cells, bomb_x  , bomb_y+1, nw, nh) + cell_is_free(game_state->cells, bomb_x+1, bomb_y+1, nw, nh);

                int total_neighbours =
                    cell_value(game_state->cells, bomb_x-1, bomb_y-1, nw, nh) + cell_value(game_state->cells, bomb_x  , bomb_y-1, nw, nh) +
                    cell_value(game_state->cells, bomb_x+1, bomb_y-1, nw, nh) + cell_value(game_state->cells, bomb_x-1, bomb_y  , nw, nh) +
                    cell_value(game_state->cells, bomb_x+1, bomb_y  , nw, nh) + cell_value(game_state->cells, bomb_x-1, bomb_y+1, nw, nh) +
                    cell_value(game_state->cells, bomb_x  , bomb_y+1, nw, nh) + cell_value(game_state->cells, bomb_x+1, bomb_y+1, nw, nh);
                
                game_state->cells[index].bomb.total = total_neighbours;
                game_state->cells[index].bomb.free_cells = free_neighbours;                            
            }

            for(int counter = 0; counter < bomb_counter - 1; ++counter)
            {
                for(int c = 0; c < bomb_counter - counter - 1; ++c)
                {
                    int index = bombs_to_update[c];
                    int index_p1 = bombs_to_update[c + 1];
                    if( (game_state->cells[index].bomb.value - game_state->cells[index].bomb.total) >
                        (game_state->cells[index_p1].bomb.value - game_state->cells[index_p1].bomb.total) )
                    {
                        // Swap
                        int temp_index = bombs_to_update[c];
                        bombs_to_update[c] = bombs_to_update[c + 1];               
                        bombs_to_update[c + 1] = temp_index;
                    }
                }
            }
            
            int potential_value = (int)game_state->cells[bombs_to_update[0]].bomb.value / game_state->cells[bombs_to_update[0]].bomb.free_cells;
            if(potential_value == 0) ++potential_value;    // Maybe value is 0.9.
            if(potential_value > (game_state->cells[bombs_to_update[0]].bomb.value - game_state->cells[bombs_to_update[0]].bomb.total))
               potential_value = game_state->cells[bombs_to_update[0]].bomb.value - game_state->cells[bombs_to_update[0]].bomb.total;
                        
            game_state->cells[i].value = potential_value;
            game_state->cells[i].flag |= MASK_TESTED;
        }
    }

    //update free_cell and total
    for(int y = 0; y < nh; ++y)
    {
        for(int x = 0; x < nw; ++x)
        {
            int index = y * nw + x;
            if(!(game_state->cells[index].flag & MASK_BOMB)) continue;
            
            int free_neighbours =
                cell_is_free(game_state->cells, x-1, y-1, nw, nh) + cell_is_free(game_state->cells, x  , y-1, nw, nh) +
                cell_is_free(game_state->cells, x+1, y-1, nw, nh) + cell_is_free(game_state->cells, x-1, y  , nw, nh) +
                cell_is_free(game_state->cells, x+1, y  , nw, nh) + cell_is_free(game_state->cells, x-1, y+1, nw, nh) +
                cell_is_free(game_state->cells, x  , y+1, nw, nh) + cell_is_free(game_state->cells, x+1, y+1, nw, nh);

            int total_neighbours =
                cell_value(game_state->cells, x-1, y-1, nw, nh) + cell_value(game_state->cells, x  , y-1, nw, nh) +
                cell_value(game_state->cells, x+1, y-1, nw, nh) + cell_value(game_state->cells, x-1, y  , nw, nh) +
                cell_value(game_state->cells, x+1, y  , nw, nh) + cell_value(game_state->cells, x-1, y+1, nw, nh) +
                cell_value(game_state->cells, x  , y+1, nw, nh) + cell_value(game_state->cells, x+1, y+1, nw, nh);

            game_state->cells[index].bomb.total = total_neighbours;
            game_state->cells[index].bomb.free_cells = free_neighbours;

            if(game_state->cells[index].bomb.value > game_state->cells[index].bomb.total)
                game_state->cells[index].bomb.value = game_state->cells[index].bomb.total;
        }
    }

#if 0
    for(int i = 0; i < game_state->grid.bombs_number; ++i)
    {
        printf("Bomb %d - index: %d, value: %d, total: %d, free_cells: %d\n", i,
               bombs_index[i], game_state->cells[bombs_index[i]].bomb.value,
               game_state->cells[bombs_index[i]].bomb.total, game_state->cells[bombs_index[i]].bomb.free_cells);
    }
#endif
}
