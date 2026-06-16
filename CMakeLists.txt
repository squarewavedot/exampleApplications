cmake_minimum_required(VERSION 3.07)
project(exampleApplications VERSION 0.4.0)


set(CMAKE_CXX_STANDARD 17)

set(Boost_NO_BOOST_CMAKE ON)
set(Boost_FIND_QUIETLY ON) #FINALLY!!!

set(Boost_USE_MULTITHREADED TRUE)

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

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/matrixserver/install" ${CMAKE_PREFIX_PATH})

find_package(PkgConfig REQUIRED)
pkg_check_modules(EIGEN3 REQUIRED eigen3)
pkg_check_modules(IMLIB2 REQUIRED imlib2)
include_directories(${EIGEN3_INCLUDE_DIRS} ${IMLIB2_INCLUDE_DIRS})
link_directories(${EIGEN3_LIBRARY_DIRS} ${IMLIB2_LIBRARY_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

IF(APPLE)
	# Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
	LINK_DIRECTORIES(/usr/local/lib)
ENDIF()


add_subdirectory(Genetic)
add_subdirectory(CubeTestApp)
add_subdirectory(Breakout3D)
add_subdirectory(Blackout3D)
add_subdirectory(PixelFlow3)
# add_subdirectory(MatrixRain)
add_subdirectory(Snake)
add_subdirectory(PortalCube)
add_subdirectory(Pacman)
add_subdirectory(AgarIO)

IF(BUILD_RASPBERRYPI)
    add_subdirectory(ImuTest)
    add_subdirectory(PixelFlow)
    add_subdirectory(PixelFlow2)
    add_subdirectory(Rainbow)
ENDIF()
#add_subdirectory(Picture)


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
