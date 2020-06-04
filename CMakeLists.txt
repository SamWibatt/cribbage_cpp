cmake_minimum_required(VERSION 3.10)

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

# add the executable
add_executable(test_main test_main.cpp)

target_link_libraries(test_main PUBLIC ${EXTRA_LIBS})
target_link_libraries(test_main PUBLIC gtest)
target_link_libraries(test_main PUBLIC pthread)

# add the binary tree to the search path for include files
# so that we will find any configured headers once we have them
target_include_directories(test_main PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${EXTRA_INCLUDES}
                           )
