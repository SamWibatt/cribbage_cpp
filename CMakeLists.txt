# rebuilding vscode version so I can debug - should also work from cmdline, yes?

cmake_minimum_required(VERSION 3.14.0)

# project settings ================================================================================

project(cribbage_cpp VERSION 0.1.0)

include(CTest)
enable_testing()



# here set flag for clang coverage testing
# maybe an option that defaults to off, can edit to ON or send something on cmdline
# something like
# cmake -DTEST_COVERAGE=ON ..
# choose LinuxCov as variant to set on in vscode
option(TEST_COVERAGE "Set ON to generate code coverage for main_test" OFF)

if(TARGET_PLATFORM MATCHES "Linux.*")
# add gtest before any coverage configuration is done bc we don't want coverage of it
# plus arduino can't do it
  add_subdirectory(googletest)
  if(TEST_COVERAGE)
    message("Compiling with test coverage ******************************")
    # see https://clang.llvm.org/docs/SourceBasedCodeCoverage.html
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)    
  endif()
  # also only linux gets the test_main - is it useful to have coverage data for these?
  # kind of useful to have coverage data for *everything* to avoid cruft, really
  add_executable(test_main test_main.cpp test_card_utils.cpp test_score_shew.cpp test_play_card.cpp test_default_player.cpp)
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

# add any post-build steps
# https://cmake.org/cmake/help/latest/command/add_custom_command.html
if(TARGET_PLATFORM MATCHES "Linux.*")
  if(TEST_COVERAGE)
    # also add the post-build commands to generate the report files
    #add_custom_command(TARGET <target>
    #PRE_BUILD | PRE_LINK | POST_BUILD
    #COMMAND command1 [ARGS] [args1...]
    #[COMMAND command2 [ARGS] [args2...] ...]
    #[BYPRODUCTS [files...]]
    #[WORKING_DIRECTORY dir]
    #[COMMENT comment]
    #[VERBATIM] [USES_TERMINAL]
    #[COMMAND_EXPAND_LISTS])
    #
    add_custom_command(TARGET test_main POST_BUILD
      # emits default.profraw
      USES_TERMINAL
      VERBATIM
      COMMAND echo "script -q -c './test_main' testresults.typescript > /dev/null" > makereport.sh 
      COMMAND llvm-profdata merge -sparse default.profraw -o testcov.profdata
      # these didn't keep the lovely colors - maybe we can just build a shell script
      #COMMAND script -q -c 'llvm-cov show ./test_main -show-line-counts-or-regions -instr-profile=testcov.profdata'  covcounts.typescript > /dev/null
      #COMMAND script -q -c 'llvm-cov report ./test_main -instr-profile=testcov.profdata' covsummary.typescript > /dev/null
      COMMAND echo "script -q -c 'llvm-cov show ./test_main -show-line-counts-or-regions -instr-profile=testcov.profdata'  covcounts.typescript > /dev/null" >> makereport.sh
      COMMAND echo "script -q -c 'llvm-cov report ./test_main -instr-profile=testcov.profdata' covsummary.typescript > /dev/null" >> makereport.sh
      COMMAND echo "llvm-cov report ./test_main -instr-profile=testcov.profdata" >> makereport.sh
      COMMAND chmod 755 makereport.sh
      # swh if we run that - strips out the ansi
      #COMMAND ./makereport.sh
      COMMAND echo "**********************************************************************************************"
      COMMAND echo "**********************************************************************************************"
      COMMAND echo "**********************************************************************************************"
      COMMAND echo "Created makereport.sh in build directory - run that to get test results and coverage in:"
      COMMAND echo "- testresults.typescript = output from unit tests"
      COMMAND echo "- covcounts.typescript = detailed source/region-level coverage output"
      COMMAND echo "- cosummary.typescript = summary coverage report"
      COMMAND echo "done this clumsy way to preserve the ANSI color output in the testing and coverage reports"
      COMMAND echo "use 'less -R <file>' or other ANSI-capable viewing mechanism to show the typescripts in color"
      COMMAND echo "**********************************************************************************************"
      COMMAND echo "**********************************************************************************************"
      COMMAND echo "**********************************************************************************************"
    )
    # LLVM_PROFILE_FILE="testcov.profraw" ./test_main 
    # llvm-profdata merge -sparse testcov.profraw -o testcov.profdata
    # # original cmdlines
    # #llvm-cov show ./test_main -show-line-counts-or-regions -instr-profile=testcov.profdata > covcounts.txt
    # #llvm-cov report ./test_main -instr-profile=testcov.profdata > covsummary.txt
    # # see if I can preserve the nice colors, which should show when using less -R, though no editors seem to
    # # see https://unix.stackexchange.com/questions/249723/how-to-trick-a-command-into-thinking-its-output-is-going-to-a-terminal
    # # script -q -c 'ls -G' out_file
    # # redirect to /dev/null or it dumps everything to the console too!
    # # output works with more and cat, too! The colors are good for quickly spotting unvisited regions.
    # script -q -c 'llvm-cov show ./test_main -show-line-counts-or-regions -instr-profile=testcov.profdata'  covcounts.typescript > /dev/null
    # script -q -c 'llvm-cov report ./test_main -instr-profile=testcov.profdata' covsummary.typescript > /dev/null
    
    # # print summary to console
    # llvm-cov report ./test_main -instr-profile=testcov.profdata
  endif()
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
