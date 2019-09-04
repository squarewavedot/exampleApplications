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
add_subdirectory(Snake)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LEDCube matrixserver example Applications")
set(CPACK_PACKAGE_DESCRIPTION "LEDCube matrixserver example Applications")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")


set(CPACK_DEBIAN_PACKAGE_DEPENDS "matrixserver")
set(CPACK_PACKAGE_CONTACT "Adrian maintainer@squarewave.io")
#set(CPACK_PACKAGING_INSTALL_PREFIX "")
#set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/Debian/postinst")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
include(CPack)