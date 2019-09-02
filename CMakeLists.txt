cmake_minimum_required(VERSION 3.07)
project(exampleApplications)


set(CMAKE_CXX_STANDARD 14)

set(Boost_FIND_QUIETLY ON) #FINALLY!!!

if (EXISTS "/boot/LICENCE.broadcom") # I expect this file to be only found on an Raspian distribution
    set(BUILD_RASPBERRYPI true)
    message("Build on Raspberry Pi is enabled")
endif ()

set(default_build_type "Release")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(Genetic)
add_subdirectory(CubeTestApp)
add_subdirectory(ImuTest)
add_subdirectory(Breakout3D)
add_subdirectory(PixelFlow)
add_subdirectory(PixelFlow3)
add_subdirectory(Picture)