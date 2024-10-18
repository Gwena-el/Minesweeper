#include <cstdio>
#include <cstdint>


struct Cell
{
    uint8_t value;
    uint8_t flag;
};

struct Grid_Data
{
    int index, nw, nh, cw, ch, pos_x, pos_y, bombs_number;
    Cell* cells;
};


// -----------------------------------------------------------------------------------------
static void make_level(char *path, char *level_name)
{
    FILE* bmp_file = fopen(path, "rb");
    if(bmp_file)
    {
        fseek(bmp_file, 0, SEEK_END);
        size_t filesize = ftell(bmp_file);
        fseek(bmp_file, 0, SEEK_SET);

        char* buffer = (char *)malloc(filesize);
        if(buffer)
        {
            fread(buffer, filesize, 1, bmp_file);
            fclose(bmp_file);

            
        }
    }
    else fprintf(stderr, "Error: could'nt find bmp image.");
}

// -----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    if(argc == 2)
    {
        char bmp_path[32];
        char output_name[32];
        sscanf(argv[1], "%s", bmp_path);
        sscanf(argv[2], "%s", output_name);
        
        make_level(bmp_path, output_name);
    }
    else fprintf(stderr, "Error: require path to a bmp image,"
                 "and an output name to produce a level file.\n");
    
    return 0;
}
