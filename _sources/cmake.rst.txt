====================
CMake Project Details
====================

This section provides detailed information about the CMake build system configuration, project structure, and build customization options for the LED Matrix Cube example applications.

Project Structure
=================

The CMake project is organized with a main project file and individual subdirectories for each application:

.. code-block:: text

   exampleApplications/
   ├── CMakeLists.txt              # Main project configuration
   ├── CubeTestApp/
   │   ├── CMakeLists.txt          # CubeTestApp build configuration
   │   ├── CubeTest.cpp
   │   ├── CubeTest.h
   │   └── main.cpp
   ├── Snake/
   │   ├── CMakeLists.txt          # Snake game build configuration
   │   ├── snake.cpp
   │   ├── snake.h
   │   └── main.cpp
   └── [other applications...]

Main CMake Configuration
========================

Project Metadata
----------------

The main ``CMakeLists.txt`` defines the project metadata and global settings:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.07)
   project(exampleApplications)
   
   set(CMAKE_CXX_STANDARD 17)

Global Build Settings
--------------------

**C++ Standard**: The project requires C++17 features and sets this as the minimum standard.

**Build Type Configuration**: Defaults to Release mode for optimal performance:

.. code-block:: cmake

   set(default_build_type "Release")
   
   if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
       message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
       set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
       set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
               "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
   endif()

**Compiler Optimization**: Release builds use aggressive optimization:

.. code-block:: cmake

   set(CMAKE_CXX_FLAGS_RELEASE "-O3")

Platform Detection
==================

Raspberry Pi Detection
---------------------

The build system automatically detects Raspberry Pi platforms:

.. code-block:: cmake

   if (EXISTS "/boot/LICENCE.broadcom")
       set(BUILD_RASPBERRYPI true)
       message("Build on Raspberry Pi is enabled")
   endif()

This detection enables:

* Platform-specific applications (ImuTest, PixelFlow, PixelFlow2, Rainbow)
* Hardware-optimized build flags
* Raspberry Pi specific library paths

macOS-Specific Configuration
---------------------------

The build system includes macOS-specific link directory configuration:

.. code-block:: cmake

   IF(APPLE)
       # Fix linking on 10.14+
       LINK_DIRECTORIES(/usr/local/lib) 
   ENDIF()

This addresses linking issues on newer macOS versions by explicitly including the Homebrew library directory.

Dependency Management
====================

matrixapplication Discovery
---------------------------

The build system searches for the ``matrixapplication`` package (version >= 0.3) using a
prioritised path list. ``./matrixserver/install/`` is checked first, allowing a locally
built matrixserver to be picked up without any extra configuration:

.. code-block:: cmake

   set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/matrixserver/install" ${CMAKE_PREFIX_PATH})

To use this, build and install matrixserver into that directory:

.. code-block:: bash

   # Inside the matrixserver repository
   cmake .. -DCMAKE_INSTALL_PREFIX=../install
   make && make install

If the package is not found locally, CMake falls back to the normal system search paths.
For a completely custom location, pass ``-DCMAKE_PREFIX_PATH=/your/path`` at configure time.

Boost Configuration
------------------

Boost libraries are configured globally with specific requirements:

.. code-block:: cmake

   set(Boost_FIND_QUIETLY ON)
   set(Boost_USE_MULTITHREADED TRUE)

The ``Boost_FIND_QUIETLY`` flag reduces verbose output during configuration, while ``Boost_USE_MULTITHREADED`` ensures thread-safe Boost libraries are linked.

Individual Application Configuration
===================================

Each application has its own ``CMakeLists.txt`` with standardized patterns:

Typical Application CMake Structure
----------------------------------

.. code-block:: cmake

   project(applicationName)
   
   # Find required packages
   find_package(Boost 1.58.0 REQUIRED COMPONENTS thread log system)
   include_directories(${Boost_INCLUDE_DIRS})
   
   # Platform-specific library detection
   find_library(IMLIB2_FOUND "Imlib2" PATHS "/opt/homebrew/Cellar/imlib2/1.12.2/")
   IF(IMLIB2_FOUND)
       MESSAGE("\tIMLIB2 Found")
       LINK_DIRECTORIES(/opt/homebrew/Cellar/imlib2/1.12.2/lib)
       INCLUDE_DIRECTORIES(/opt/homebrew/Cellar/imlib2/1.12.2/include)
   ELSE(IMLIB2_FOUND)
       MESSAGE(FATAL_ERROR "\tIMLIB2 Not Found")
   ENDIF(IMLIB2_FOUND)
   
   # Additional required packages

   find_package(matrixapplication 0.4 REQUIRED)
   
   # Define executable and link libraries
   add_executable(applicationName source_files...)
   target_link_libraries(applicationName 
       matrixapplication::matrixapplication 
   )

Library Detection Patterns
--------------------------

**Imlib2 Detection**: The applications use custom detection for Imlib2, especially on macOS with Homebrew:

.. code-block:: cmake

   find_library(IMLIB2_FOUND "Imlib2" PATHS "/opt/homebrew/Cellar/imlib2/1.12.2/")
   IF(IMLIB2_FOUND)
       MESSAGE("\tIMLIB2 Found")
       LINK_DIRECTORIES(/opt/homebrew/Cellar/imlib2/1.12.2/lib)
       INCLUDE_DIRECTORIES(/opt/homebrew/Cellar/imlib2/1.12.2/include)
   ELSE(IMLIB2_FOUND)
       MESSAGE(FATAL_ERROR "\tIMLIB2 Not Found")
   ENDIF(IMLIB2_FOUND)

**Framework Integration**: All applications link against the matrixapplication library:

.. code-block:: cmake

   find_package(matrixapplication 0.4 REQUIRED)
   target_link_libraries(applicationName matrixapplication::matrixapplication)

Application Selection
====================

Conditional Compilation
-----------------------

The main CMakeLists.txt conditionally includes applications based on platform and configuration:

.. code-block:: cmake

   # Always enabled applications
   add_subdirectory(Genetic)
   add_subdirectory(CubeTestApp)
   add_subdirectory(Breakout3D)
   add_subdirectory(Blackout3D)
   add_subdirectory(PixelFlow3)
   add_subdirectory(MatrixRain)
   add_subdirectory(Snake)
   
   # Raspberry Pi specific applications
   IF(BUILD_RASPBERRYPI)
       add_subdirectory(ImuTest)
       add_subdirectory(PixelFlow)
       add_subdirectory(PixelFlow2)
       add_subdirectory(Rainbow)
   ENDIF()
   
   # Picture application (currently disabled)
   #add_subdirectory(Picture)

This configuration allows for:

* **Selective compilation** of applications
* **Platform-appropriate builds** (Raspberry Pi vs desktop)
* **Easy enabling/disabling** of specific applications by uncommenting lines

Package Generation
=================

CPack Configuration
------------------

The project includes comprehensive CPack configuration for generating distribution packages:

.. code-block:: cmake

   set(CPACK_GENERATOR "DEB")
   set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LEDCube matrixserver example Applications")
   set(CPACK_PACKAGE_DESCRIPTION "LEDCube matrixserver example Applications")
   
   set(CPACK_DEBIAN_PACKAGE_DEPENDS "matrixserver")
   set(CPACK_PACKAGE_CONTACT "Adrian maintainer@squarewave.io")
   
   set(CPACK_PACKAGE_VERSION_MAJOR "0")
   set(CPACK_PACKAGE_VERSION_MINOR "1")
   set(CPACK_PACKAGE_VERSION_PATCH "0")
   set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
   
   include(CPack)

This configuration:

* **Generates Debian packages** (.deb files)
* **Declares dependency** on the matrixserver package
* **Sets package metadata** including version and maintainer
* **Uses default naming** for generated package files

Build Targets
=============

Standard Targets
---------------

The CMake configuration provides several standard targets:

.. code-block:: bash

   # Build all enabled applications
   make
   
   # Build specific application
   make cubetestapp
   make snake
   make pixelflow3
   
   # Generate distribution package
   make package
   
   # Clean build artifacts
   make clean

Custom Build Options
===================

Enabling Additional Applications
-------------------------------

To enable commented-out applications, edit the main ``CMakeLists.txt``:

.. code-block:: cmake

   # Uncomment desired applications
   add_subdirectory(Genetic)
   add_subdirectory(Breakout3D)
   add_subdirectory(Blackout3D)
   add_subdirectory(Snake)

Custom Library Paths
--------------------

For non-standard library installations, use CMake variables:

.. code-block:: bash

   # Use a locally installed matrixserver (alternative to ./matrixserver/install/)
   cmake -DCMAKE_PREFIX_PATH=/path/to/matrixserver/install ..
   
   # Custom Boost installation
   cmake -DBOOST_ROOT=/custom/boost/path ..
   
   # Custom Imlib2 installation  
   cmake -DIMLIB2_ROOT=/custom/imlib2/path ..
   
   # Custom installation prefix
   cmake -DCMAKE_INSTALL_PREFIX=/custom/install/path ..

Debug Configuration
------------------

For development and debugging:

.. code-block:: bash

   # Debug build with symbols
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   
   # Verbose build output
   make VERBOSE=1

Build System Maintenance
=======================

Adding New Applications
----------------------

To add a new application to the build system:

1. **Create application directory** with source files
2. **Add CMakeLists.txt** following the standard pattern
3. **Update main CMakeLists.txt** to include the new subdirectory:

   .. code-block:: cmake

      add_subdirectory(NewApplication)

Dependency Updates
-----------------

When updating dependencies:

1. **Update version requirements** in ``find_package`` calls
2. **Test compilation** across all target platforms  
3. **Update documentation** to reflect new requirements

This CMake configuration provides a robust, cross-platform build system that automatically adapts to different environments while maintaining consistent behavior and easy customization.