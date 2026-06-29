cmake_minimum_required(VERSION 3.20)
project(infusion_test)
set(CMAKE_CXX_STANDARD 20)
find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

add_executable(infusion_test
    test_infusion.cpp
    mock_hardware.cpp
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
    GTest::GTest
    GTest::Main
)
add_custom_target(coverage
    COMMAND lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
    COMMAND lcov --remove coverage.info '/usr/*' --output-file coverage_filtered.info --ignore-errors unused
    COMMAND genhtml coverage_filtered.info --output-directory coverage_html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)   