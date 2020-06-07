cmake_minimum_required(VERSION 3.10)

# OK, going to ignore VS code build system....
# here set platform
# possible values are:
# LinuxConsole
# LinuxAllegro
# ArduinoSerial
# ArduinoTFT
# can set from cmdline with cmake -DTARGET_PLATFORM=xxx
# default to linux console 
if(NOT DEFINED TARGET_PLATFORM)
    set (TARGET_PLATFORM "ArduinoSerial")  # "LinuxConsole")
endif()

# so we can do stuff like check to see if it's a Linux of any kind with 
#if(<variable|string> MATCHES regex)
# which can even do captures into CMAKE_MATCH_n for n = 0..9
# or equal to an exact target with 
#if(<variable|string> STREQUAL <variable|string>)

# pre-project stuff 
# see if I can set the arduino install path here - it looks like the project command is what invokes
# the chain of stuff that dies on this not being defined
# THIS SEEMS TO WORK
if(TARGET_PLATFORM MATCHES "Arduino.*")
    set(ARDUINO_INSTALL_PATH "/home/sean/software/arduino-1.8.12/")
    # and board - copied this line from build/BoardOptions.cmake after initial run without it set
    set(ARDUINO_BOARD "Adafruit Circuit Playground Express [samd.adafruit_circuitplayground_m0]") # Adafruit Circuit Playground Express
    # HERE ALSO NEED TO SET THE toolchain?
    set(CMAKE_TOOLCHAIN_FILE "Arduino-CMake-Toolchain/Arduino-toolchain.cmake")
elseif(TARGET_PLATFORM MATCHES "Linux.*")
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
else()
    # error! Unknown platform. die
endif()

# set the project name and version
project(cribbage_cpp VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add platform-independent libraries
add_subdirectory(cribbage_core)
list(APPEND EXTRA_LIBS CardUtils)
list(APPEND EXTRA_LIBS Cribbage)
list(APPEND EXTRA_LIBS CribbagePlayer)

# and platform-independent includes
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/cribbage_core")

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
        list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/plat_linux_console")
    elseif(TARGET_PLATFORM STREQUAL "ArduinoTFT")
        add_subdirectory("${PROJECT_SOURCE_DIR}/plat_linux_allegro")
        list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/plat_linux_allegro")
    endif()
endif()

# add the platspec libraries 
list(APPEND EXTRA_LIBS PlatPlayMain)
list(APPEND EXTRA_LIBS PlatIO)


# add libs for play_main 
target_link_libraries(play_main PUBLIC ${EXTRA_LIBS})

target_include_directories(play_main PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${EXTRA_INCLUDES}
                           )

# test only for linux atm - at least they'll be different
if(TARGET_PLATFORM MATCHES "Linux*")
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
endif()