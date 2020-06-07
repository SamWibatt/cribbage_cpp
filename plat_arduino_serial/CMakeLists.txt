add_library(PlatPlayMain plat_play_main.cpp)
target_link_arduino_libraries(PlatPlayMain PRIVATE core)

add_library(PlatIO plat_io.cpp)
target_link_arduino_libraries(PlatIO PRIVATE core)
