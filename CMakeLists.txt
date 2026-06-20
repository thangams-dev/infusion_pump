cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(infusion_pump)

target_sources(app PRIVATE
    main.cpp
    src/alarm.cpp
    src/occlusion.cpp
    src/volume.cpp
    src/infusion.cpp
    src/hardware.cpp
)

target_include_directories(app PRIVATE include)