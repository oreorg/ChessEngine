cmake_minimum_required(VERSION 3.16)

project(ChessEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_executable(ChessEngine ${SOURCES}
    include/chess/engine/magic.h
    include/chess/core/helper.h
    include/chess/core/transposition_table.h
    src/core/transposition_table.cpp)

target_include_directories(ChessEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(SFML 3.0 REQUIRED COMPONENTS Graphics Window System)
# Copy the folder after the executable is built
add_custom_command(
    TARGET ChessEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:ChessEngine>/assets"
)

target_link_libraries(ChessEngine PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System
)

if(MSVC)
    target_compile_options(ChessEngine PRIVATE /O2 /W4)
else()
    target_compile_options(ChessEngine PRIVATE -O3 -Wall -Wextra)
endif()