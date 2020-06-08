cmake_minimum_required(VERSION 3.14)        # add_link_options needs 3.13+

# PRE-PROJECT SETTINGS ==================================================================================

# here set platform
# possible values are: LinuxConsole LinuxAllegro ArduinoSerial ArduinoTFT
# can set from cmdline with cmake -DTARGET_PLATFORM=xxx
# can set default here 
if(NOT DEFINED TARGET_PLATFORM)
    set (TARGET_PLATFORM "LinuxConsole")
endif()

# sanity check platform
if(NOT TARGET_PLATFORM STREQUAL "LinuxConsole" AND
   NOT TARGET_PLATFORM STREQUAL "LinuxAllegro" AND
   NOT TARGET_PLATFORM STREQUAL "ArduinoSerial" AND
   NOT TARGET_PLATFORM STREQUAL "ArduinoTFT")
   message(FATAL_ERROR "Unrecognized TARGET_PLATFORM")
endif()

# here set flag for clang coverage testing
# maybe an option that defaults to off, can edit to ON or send something on cmdline
# something like
# cmake -DTEST_COVERAGE=ON ..
option(TEST_COVERAGE "Set ON to generate code coverage for main_test" OFF)

# set arduino install path here for the Arduino CMake Toolchain
# MAYBE PUT THIS SOMEWHERE MORE OBVIOUS - can be up top, no harm in setting it if the
# compile isn't for arduino, yes?
# also the path isn't necessary if the installation is in a standard place - see the submodule
# write up how to do this!
if(TARGET_PLATFORM MATCHES "Arduino.*")
    set(ARDUINO_INSTALL_PATH "/home/sean/software/arduino-1.8.12/")
    # and board - copied this line from build/BoardOptions.cmake after initial run without it set
    set(ARDUINO_BOARD "Adafruit Circuit Playground Express [samd.adafruit_circuitplayground_m0]") # Adafruit Circuit Playground Express
    # Select the Arduino-CMake-Toolchain toolchain
    set(CMAKE_TOOLCHAIN_FILE "Arduino-CMake-Toolchain/Arduino-toolchain.cmake")
elseif(TARGET_PLATFORM MATCHES "Linux.*")
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()


# PROJECT NAME AND POST-PROJECT SETTINGS ================================================================

# set the project name and version
project(cribbage_cpp VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add/compile google test before the coverage compile options to leave it out of the report
# YAY THIS WORKS
if(TARGET_PLATFORM MATCHES "Linux*")
    # add google test submod - LATER NOT LINUX ONLY BUT THE COVERAGE WILL BE
    add_subdirectory(googletest)

    # THEN can do the test coverage part
    if(TEST_COVERAGE)
        message("Compiling with test coverage")
        # OK so in here is where we'd set up the clang code coverage options if we want that, yes?
        # see https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
        # clang++ -fprofile-instr-generate -fcoverage-mapping foo.cc -o foo
        # SEE HOW TO SET COMPILER OPTIONS - https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_FLAGS.html
        # This is initialized for each language from environment variables:
        # CMAKE_C_FLAGS: Initialized by the CFLAGS environment variable.
        # CMAKE_CXX_FLAGS: Initialized by the CXXFLAGS environment variable. 
        # BETTER: use add_compile_options so as not to trash what's already in there
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        # per https://stackoverflow.com/questions/56112019/clang-does-not-generate-profraw-file-when-linking-manually
        # they also need to be added to the link step
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    endif()   
endif()


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
# SOON MAYBE WE'LL HAVE GTEST FOR ARDUINO YAY
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