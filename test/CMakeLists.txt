#Required version
cmake_minimum_required(VERSION 3.20)

#project name
project(infusion_test)

#Requires C++20 , Forces to follow this
set(CMAKE_CXX_STANDARD 20)

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

add_executable(infusion_test
    test_infusion.cpp
    stub_file.cpp
    ../src/alarm.cpp
    ../src/occlusion.cpp
    ../src/volume.cpp
    ../src/infusion.cpp
)

target_include_directories(infusion_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(infusion_test PRIVATE UNIT_TEST)

target_link_libraries(infusion_test
    gtest
    gtest_main
)

add_test(NAME infusion_test COMMAND infusion_test)

add_custom_target(coverage
    COMMAND lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch --rc branch_coverage=1
    COMMAND lcov --remove coverage.info '/usr/*' '*/test_infusion.cpp' '*/stub_file.cpp' --output-file coverage_filtered.info --ignore-errors unused --rc branch_coverage=1
    COMMAND genhtml coverage_filtered.info --output-directory coverage_html --rc branch_coverage=1
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)