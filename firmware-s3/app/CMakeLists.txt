# Firmware unico do console: launcher (menu) + Tetris + Doom. Os projetos em examples/
# (tetris_standalone, doom_standalone) continuam existindo para depurar cada jogo isolado.
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../components")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(portatil)
