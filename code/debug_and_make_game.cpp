// -----------------------------------------------------------------------------------------
global_variable int max_file;
global_variable int file_counter = 0;
internal void DEBUG_save_level(Grid *grid_head)
{    
    char filename[64];
    sprintf(filename, "../data/level%d.txt", file_counter);
    FILE* file = fopen(filename, "w");

    if(file)
    {
        Grid* current = grid_head;
        while(current)
        {
            fprintf(file, "index:%d\n", current->index);
            fprintf(file, "nw:%d\n", current->n_cells.w);
            fprintf(file, "nh:%d\n", current->n_cells.h);
            fprintf(file, "cw:%d\n", current->cell.w);
            fprintf(file, "ch:%d\n", current->cell.h);
            fprintf(file, "pos_x:%d\n", current->pos.x);
            fprintf(file, "pos_y:%d\n", current->pos.y);
            fprintf(file, "bombs:%d\n", current->bombs_number);
            fprintf(file, "\n");

            current = current->next;
        }
        
        file_counter++;
        fclose(file);        
    }
}


global_variable bool debug_can_click = true;
global_variable int debug_timer = 0;
global_variable bool lock_drag = false;
#define DEBUG_FONT_SCALE 16
// -----------------------------------------------------------------------------------------
internal void DEBUG_drag_grid(Game_State *game_state, UI_Context *ui_ctx, Assets *assets)
{
    int temp_w, temp_h;
    
    max_file = game_state->parkour_data.number_of_levels;
    if(game_state->mode == PARKOUR)
    {
        local_persist Grid* focused;
        local_persist Grid* current = game_state->grid;
        while(current)
        {
            if(Point_In_Rect(ui_ctx->mouse.x, ui_ctx->mouse.y, current->pos.x, current->pos.y, current->size.w, current->size.h))
            {
                if(!lock_drag)
                {
                    focused = current;
                }
                break;   
            }
            else
                current = current->next;
        }

        char file_counter_char[64];
        sprintf(file_counter_char, "max level: %d\nslct_level: %d\ncurrent_level: %d", max_file, file_counter, game_state->parkour_data.load_level);
        Get_String_Size(file_counter_char, DEBUG_FONT_SCALE, &temp_w, &temp_h);
        Make_String(assets->ui_layer, assets->font_bitmap, file_counter_char, DEBUG_FONT_SCALE,
                    300, game_state->screen_size.h - 64, temp_w, temp_h);

        const Uint8 *state = SDL_GetKeyboardState(NULL);
        bool ctrl_pressed = false;
        if(state[SDL_SCANCODE_CAPSLOCK]) ctrl_pressed = true;

        // Increment file counter ( to save to new location )
        if(state[SDL_SCANCODE_L] && ctrl_pressed && debug_can_click)
        {
            debug_can_click = false;
            file_counter++;
            if(file_counter > max_file) max_file = file_counter;
        }
        // Decrement file counter ( to replace existing file )
        if(state[SDL_SCANCODE_M] && ctrl_pressed && debug_can_click)
        {
            debug_can_click = false;
            if(file_counter > 0) file_counter--;
        }

        // Save to file
        if(state[SDL_SCANCODE_S] && ctrl_pressed && debug_can_click)
        {
            debug_can_click = false;
            DEBUG_save_level(game_state->grid);
            if(file_counter > max_file) game_state->parkour_data.number_of_levels = file_counter;
        }

        // Add Grid
        if(state[SDL_SCANCODE_A] && ctrl_pressed && debug_can_click)
        {
            debug_can_click = false;
            Add_Grid(&game_state->grid);

            Grid* current = game_state->grid;
            while(current->next) current = current->next;
            Grid_Update(current, current->index, 2, 2, 32, 32, ui_ctx->mouse.x, ui_ctx->mouse.y, 1);

            New_Game_Parkour(game_state,0,0,0);                
        }

        // Go to next level
        if(state[SDL_SCANCODE_Q] && ctrl_pressed && debug_can_click)
        {
            debug_can_click = false;
            if(game_state->parkour_data.load_level < game_state->parkour_data.number_of_levels-1)
            {
                game_state->parkour_load_new_level = true;
                game_state->parkour_data.load_level++;
                New_Game_Parkour(game_state, 0,0,0);   
            }
        }
        // Go to previous level
        if(state[SDL_SCANCODE_W] && ctrl_pressed && debug_can_click)
        {
            debug_can_click = false;
            if(game_state->parkour_data.load_level > 0)
            {  
                game_state->parkour_load_new_level = true;
                game_state->parkour_data.load_level--;
                New_Game_Parkour(game_state, 0,0,0);   
            }
        }
        
        // ------------------------------------------------------------------
        if(!ui_ctx->mouse.left_button.release)
        {
            const Uint8 *state = SDL_GetKeyboardState(NULL);

            // Make focused grid the current one.
            if(state[SDL_SCANCODE_F] && debug_can_click)
            {
                debug_can_click = false;
                game_state->current_grid = focused;

                game_state->grid_need_update = true;
            }

            // Delete Grid
            if(state[SDL_SCANCODE_S] && debug_can_click)
            {
                debug_can_click = false;
                Delete_Grid(&game_state->grid, focused->index);

                New_Game_Parkour(game_state,0,0,0);                
            }
            
            // Increase width
            if(state[SDL_SCANCODE_RIGHT] && debug_can_click)
            {
                debug_can_click = false;
                focused->n_cells.w++;
                
                Grid_Update(focused, focused->index, focused->n_cells.w, focused->n_cells.h,
                            focused->cell.w, focused->cell.h, focused->pos.x, focused->pos.y,
                            focused->bombs_number);

                New_Game_Parkour(game_state,0,0,0);                
            }

            // Increase height
            if(state[SDL_SCANCODE_DOWN] && debug_can_click)
            {
                debug_can_click = false;
                focused->n_cells.h++;
                
                Grid_Update(focused, focused->index, focused->n_cells.w, focused->n_cells.h,
                            focused->cell.w, focused->cell.h, focused->pos.x, focused->pos.y,
                            focused->bombs_number);

                New_Game_Parkour(game_state,0,0,0);                
            }

            // Increase Bombs number
            if(state[SDL_SCANCODE_I] && debug_can_click)
            {
                debug_can_click = false;
                focused->bombs_number++;
                
                Grid_Update(focused, focused->index, focused->n_cells.w, focused->n_cells.h,
                            focused->cell.w, focused->cell.h, focused->pos.x, focused->pos.y,
                            focused->bombs_number);

                New_Game_Parkour(game_state,0,0,0);                
            }

            // Decrease width
            if(state[SDL_SCANCODE_LEFT] && debug_can_click)
            {
                debug_can_click = false;
                if(focused->n_cells.w > 0) focused->n_cells.w--;
                
                Grid_Update(focused, focused->index, focused->n_cells.w, focused->n_cells.h,
                            focused->cell.w, focused->cell.h, focused->pos.x, focused->pos.y,
                            focused->bombs_number);

                New_Game_Parkour(game_state,0,0,0);                
            }

            // Decrease Heihgt
            if(state[SDL_SCANCODE_UP] && debug_can_click)
            {
                debug_can_click = false;
                if(focused->n_cells.h > 0) focused->n_cells.h--;
                
                Grid_Update(focused, focused->index, focused->n_cells.w, focused->n_cells.h,
                            focused->cell.w, focused->cell.h, focused->pos.x, focused->pos.y,
                            focused->bombs_number);

                New_Game_Parkour(game_state,0,0,0);                
            }

            // Decrease Bombs number
            if(state[SDL_SCANCODE_D] && debug_can_click)
            {
                debug_can_click = false;
                focused->bombs_number--;
                
                Grid_Update(focused, focused->index, focused->n_cells.w, focused->n_cells.h,
                            focused->cell.w, focused->cell.h, focused->pos.x, focused->pos.y,
                            focused->bombs_number);

                New_Game_Parkour(game_state,0,0,0);                
            }
            
            // Drag grid and print pos ( x, y )
            if(state[SDL_SCANCODE_CAPSLOCK])
            {
                lock_drag = true;
                
                int drag_x = 0;
                int drag_y = 0;
                if(ui_ctx->mouse.x < game_state->screen_size.w && ui_ctx->mouse.y < game_state->screen_size.h)
                {
                    drag_x = ui_ctx->mouse.x - (focused->pos.x + (focused->size.w/2));
                    drag_y = ui_ctx->mouse.y - (focused->pos.y + (focused->size.h/2));   
                }
                
                if(drag_x > 100) drag_x = 1;
                if(drag_y > 100) drag_y = 1;
               
                if(focused->pos.x + drag_x < game_state->screen_size.w)
                    focused->pos.x += drag_x;
                if(focused->pos.y + drag_y < game_state->screen_size.h)
                    focused->pos.y += drag_y;


                // char temp[32];
                // sprintf(temp, "pos x : %d\npos y : %d", focused->pos.x, focused->pos.y);
                // Get_String_Size(temp, DEBUG_FONT_SCALE, &temp_w, &temp_h);
                // Make_String(assets->ui_layer, assets->font_bitmap,
                //             temp, 16, 300, 600, temp_w, temp_h);

                
                game_state->grid_need_update = true;                
            }            
        }
        else 
        {
            current = game_state->grid;
            lock_drag = false;
        }


        if(debug_timer++ > 50)
        {
            debug_timer = 0;
            debug_can_click = true;
        }
    }
}
