#!/usr/bin/env bash
# This creates a unit test coverage report by building the linux console version with 
# clang source level coverage; generates detailed report (covcounts.txt) and summary (covsummary.txt)
# report files, also sends summary to the console where it will have nice colors
# RUN FROM THIS DIRECTORY

rm -rf build_tcov
mkdir build_tcov
cd build_tcov
cmake -DTARGET_PLATFORM=LinuxConsole -DTEST_COVERAGE=ON ..
make
LLVM_PROFILE_FILE="testcov.profraw" ./test_main 
llvm-profdata merge -sparse testcov.profraw -o testcov.profdata
# original cmdlines
#llvm-cov show ./test_main -show-line-counts-or-regions -instr-profile=testcov.profdata > covcounts.txt
#llvm-cov report ./test_main -instr-profile=testcov.profdata > covsummary.txt
# see if I can preserve the nice colors, which should show when using less -R, though no editors seem to
# see https://unix.stackexchange.com/questions/249723/how-to-trick-a-command-into-thinking-its-output-is-going-to-a-terminal
# script -q -c 'ls -G' out_file
# redirect to /dev/null or it dumps everything to the console too!
# output works with more and cat, too! The colors are good for quickly spotting unvisited regions.
script -q -c 'llvm-cov show ./test_main -show-line-counts-or-regions -instr-profile=testcov.profdata'  covcounts.typescript > /dev/null
script -q -c 'llvm-cov report ./test_main -instr-profile=testcov.profdata' covsummary.typescript > /dev/null

# print summary to console
llvm-cov report ./test_main -instr-profile=testcov.profdata
