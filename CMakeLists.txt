cmake_minimum_required(VERSION 3.16)
project(MarioTerminal CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(mario
    src/main.cpp
    src/Entity.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Tile.cpp
    src/Level.cpp
)

# ncurses - available by default on macOS
find_library(CURSES_LIB NAMES ncurses curses)
target_link_libraries(mario ${CURSES_LIB})

target_compile_options(mario PRIVATE -Wall -O2)
