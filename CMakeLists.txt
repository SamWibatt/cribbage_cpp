cmake_minimum_required(VERSION 3.10)

# compiler - this needs to be before project()
# though this shouldn't be necessary w/kits
#set(CMAKE_C_COMPILER "/usr/bin/clang")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

# set the project name and version
project(cribbage_cpp VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add libraries
add_subdirectory(cribbage_core)
list(APPEND EXTRA_LIBS CardUtils)
list(APPEND EXTRA_LIBS Cribbage)
list(APPEND EXTRA_LIBS CribbagePlayer)

# and includes
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/cribbage_core")

# add the game executable
# later to be switched by platform config of some sort
add_executable(play_main play_main.cpp)
# and its libs 
target_link_libraries(play_main PUBLIC ${EXTRA_LIBS})


# add the test executable
add_executable(test_main test_main.cpp)
# and ITS libs 
target_link_libraries(test_main PUBLIC ${EXTRA_LIBS})
target_link_libraries(test_main PUBLIC gtest)
target_link_libraries(test_main PUBLIC pthread)

# add the binary tree to the search path for include files
# so that we will find any configured headers once we have them
target_include_directories(test_main PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${EXTRA_INCLUDES}
                           )

target_include_directories(play_main PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${EXTRA_INCLUDES}
                           )
