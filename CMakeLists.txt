#Current version
cmake_minimum_required(VERSION 3.20)

#It locates the library, where you download the files
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#Name of the project
project(infusion_pump)

#Adds your firmware source files to Zephyr's app target,
#This is what gets compiled and flashed to the NUCLEO board.
target_sources(app PRIVATE
    src/main.cpp
    src/alarm.cpp
    src/occlusion.cpp
    src/volume.cpp
    src/infusion.cpp
    src/hardware.cpp
)

#It will include all the headers
target_include_directories(app PRIVATE include)

# Bare-metal C++: no exceptions, no RTTI, no libstdc++ atexit/heap machinery
target_compile_options(app PRIVATE
    -fno-exceptions
    -fno-rtti
    -fno-use-cxa-atexit
)