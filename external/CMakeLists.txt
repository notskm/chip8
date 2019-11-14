project(tinyfiledialogs C)
add_library(
    tinyfiledialogs STATIC tinyfiledialogs/tinyfiledialogs.c
                           tinyfiledialogs/tinyfiledialogs.h
)

target_include_directories(tinyfiledialogs PUBLIC tinyfiledialogs)
