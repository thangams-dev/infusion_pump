#Required version
cmake_minimum_required(VERSION 3.20)

#project name
project(infusion_test)

#Requires C++20 , Forces to follow this
set(CMAKE_CXX_STANDARD 20)

#It Locates GTest library on your system, fails build if not installed.
find_package(GTest REQUIRED)

#Compile flag adds counters to code. 
#Link flag saves those counts to a file when you run it.
#Need both, or coverage data never gets written.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

#Builds one binary from test file + stub (fakes hardware) + your actual source files. 
#This is why testing real (.cpp) logic works on host without STM32 board.
add_executable(infusion_test
    test_infusion.cpp
    stub_file.cpp
    ../src/alarm.cpp
    ../src/occlusion.cpp
    ../src/volume.cpp
    ../src/infusion.cpp
)

#It Tells compiler where to find .hpp headers (include/ and current test dir).
target_include_directories(infusion_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}
)
# This is to tell, while testing use #ifdef UNIT_TEST ... #include "stub_file.hpp"
 #else ... #include "hardware.hpp" to run with fake hardware
target_compile_definitions(infusion_test PRIVATE UNIT_TEST)

#Links GTest library + its main() (so don't need to write own main).
target_link_libraries(infusion_test
    GTest::GTest
    GTest::Main
)

# "covergae" keyword to run this all commands below(Makefile)
add_custom_target(coverage

    COMMAND lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch --rc branch_coverage=1
    COMMAND lcov --remove coverage.info '/usr/*' '*/test_infusion.cpp' '*/stub_file.cpp' --output-file coverage_filtered.info --ignore-errors unused --rc branch_coverage=1
    COMMAND genhtml coverage_filtered.info --output-directory coverage_html --rc branch_coverage=1
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
) 