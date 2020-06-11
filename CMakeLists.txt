# rebuilding vscode version so I can debug - should also work from cmdline, yes?

cmake_minimum_required(VERSION 3.14.0)

# here set flag for clang coverage testing
# maybe an option that defaults to off, can edit to ON or send something on cmdline
# something like
# cmake -DTEST_COVERAGE=ON ..

# THIS IS NOT WORKING IN VSCODE
option(TEST_COVERAGE "Set ON to generate code coverage for main_test" OFF)

if(TARGET_PLATFORM MATCHES "Linux.*")
  if(TEST_COVERAGE)
    message("Compiling with test coverage")
    # see https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
  endif()
endif()


# project settings ================================================================================

project(cribbage_cpp VERSION 0.1.0)

include(CTest)
enable_testing()


# add gtest before any coverage configuration is done bc we don't want coverage of it
# plus arduino can't do it
if(TARGET_PLATFORM MATCHES "Linux.*")
  add_executable(test_main test_main.cpp test_card_utils.cpp test_score_shew.cpp test_play_card.cpp test_default_player.cpp)
  add_subdirectory(googletest)
endif()

# add platform-independent includes and libraries
# add includes 
list(APPEND EXTRA_INCLUDES "cribbage_core")
add_subdirectory(cribbage_core)
list(APPEND EXTRA_LIBS CardUtils)
list(APPEND EXTRA_LIBS Cribbage)
list(APPEND EXTRA_LIBS CribbagePlayer)

# platform-dependent includes and libraries
# add the game executable
# later to be switched by platform config of some sort
add_executable(play_main play_main.cpp)

# platform specific libraries and includes 
if(TARGET_PLATFORM MATCHES "Arduino*")
    if(TARGET_PLATFORM STREQUAL "ArduinoSerial")
        add_subdirectory("${PROJECT_SOURCE_DIR}/plat_arduino_serial")
        list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/plat_arduino_serial")
    elseif(TARGET_PLATFORM STREQUAL "ArduinoTFT")
        add_subdirectory("${PROJECT_SOURCE_DIR}/plat_arduino_tft")
        list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/plat_arduino_tft")
    endif()
    target_link_arduino_libraries(play_main PRIVATE core)
    # OTHER LIBRARIES LIKE MCUFRIEND OR WHATEVER GO HERE 
    # target for making it generate hex 
    target_enable_arduino_upload(play_main)
elseif(TARGET_PLATFORM MATCHES "Linux*")
    if(TARGET_PLATFORM STREQUAL "LinuxConsole")
        add_subdirectory("${PROJECT_SOURCE_DIR}/plat_linux_console")
        message("Hey about to add plat linux console")
        list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/plat_linux_console")
    elseif(TARGET_PLATFORM STREQUAL "ArduinoTFT")
        add_subdirectory("${PROJECT_SOURCE_DIR}/plat_linux_allegro")
        list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/plat_linux_allegro")
    endif()
endif()

# add the platspec libraries 
list(APPEND EXTRA_LIBS PlatIO)

# add includes and libraries to each project
if(TARGET_PLATFORM MATCHES "Linux.*")
  target_include_directories(test_main PUBLIC ${EXTRA_INCLUDES})
  target_link_libraries(test_main PUBLIC ${EXTRA_LIBS})
  target_link_libraries(test_main PUBLIC gtest)
  target_link_libraries(test_main PUBLIC pthread)
endif()

target_include_directories(play_main PUBLIC ${EXTRA_INCLUDES})
target_link_libraries(play_main PUBLIC ${EXTRA_LIBS})
target_link_libraries(play_main PUBLIC PlatPlayMain)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
