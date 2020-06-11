# rebuilding vscode version so I can debug - should also work from cmdline, yes?

cmake_minimum_required(VERSION 3.14.0)
project(cribbage_cpp VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(test_main test_main.cpp test_card_utils.cpp test_score_shew.cpp test_play_card.cpp test_default_player.cpp)

# add platform-independent includes and libraries
# add includes 
list(APPEND EXTRA_INCLUDES "cribbage_core")
add_subdirectory(cribbage_core)
list(APPEND EXTRA_LIBS CardUtils)
list(APPEND EXTRA_LIBS Cribbage)
list(APPEND EXTRA_LIBS CribbagePlayer)

# platform-dependent includes and libraries
# which includes gtest, bc ardy can't do that
add_subdirectory("plat_linux_console")
list(APPEND EXTRA_INCLUDES "plat_linux_console")
list(APPEND EXTRA_LIBS "PlatIO")
add_subdirectory(googletest)

target_include_directories(test_main PUBLIC ${EXTRA_INCLUDES})
target_link_libraries(test_main PUBLIC ${EXTRA_LIBS})
target_link_libraries(test_main PUBLIC gtest)
target_link_libraries(test_main PUBLIC pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
