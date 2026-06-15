cmake_minimum_required(VERSION 3.20)
project(infusion_test)
set(CMAKE_CXX_STANDARD 20)
find_package(GTest REQUIRED)

add_executable(infusion_test
    test_infusion.cpp
    stub_hardware.cpp
)

target_include_directories(infusion_test PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(infusion_test PRIVATE UNIT_TEST)

target_link_libraries(infusion_test
    GTest::GTest
    GTest::Main
)