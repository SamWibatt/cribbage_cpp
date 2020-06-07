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
llvm-cov show ./test_main -instr-profile=testcov.profdata > covcounts.txt
llvm-cov report ./test_main -instr-profile=testcov.profdata > covsummary.txt
llvm-cov report ./test_main -instr-profile=testcov.profdata
