#!/bin/bash

./tools/ctime -begin ./tools/timings_file_for_this_build.ctm

# Pass false as argument to only compile without running.
build_and_run=$1

sdl_special="-lSDL2main -lSDL2"

compiler_flags="-DINFO_PRINT_FPS=0 -DFORCE_HZ_30=0"
warnings="-Wall -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-switch"
#-Werror

cd ./tools
if g++ -g $sdl_special color_palette.cpp -o color_palette ;
then
    ./color_palette ;
fi

cd ..
if g++ -g -o0 $warnings $compiler_flags $sdl_special sdl_minesweeper.cpp -o ../build/Minesweeper ;   
then
    ./tools/ctime -end ./tools/timings_file_for_this_build.ctm
    #./ctime -stats timings_file_for_this_build.ctm
    if $build_and_run
       then 
	   cd ../build 
	   ./Minesweeper ;
    fi
fi
