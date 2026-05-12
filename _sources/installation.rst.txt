============
Installation
============

This guide covers the complete installation process for the LED Matrix Cube example applications, including dependencies, compilation, and deployment across different platforms.

Prerequisites
=============

Framework Dependency
--------------------

The example applications require the matrixserver framework to be installed first. The matrixserver provides the core libraries and runtime components needed by all applications.

**Required**: matrixserver framework with ``matrixapplication`` library (version >= 0.3)

System Dependencies
------------------

The following system libraries are required for compilation:

**Essential Libraries**

* **Boost** (version 1.58.0 or later)
  
  * ``boost-thread`` - Multi-threading support
  * ``boost-log`` - Logging framework  
  * ``boost-system`` - System utilities

* **Imlib2** - Image processing library for picture display functionality



**Platform-Specific Dependencies**

On **macOS** with Homebrew:

.. code-block:: bash

   brew install boost imlib2

On **Ubuntu/Debian**:

.. code-block:: bash

   sudo apt-get update
   sudo apt-get install libboost-all-dev libimlib2-dev

On **Raspberry Pi**:

.. code-block:: bash

   sudo apt-get update
   sudo apt-get install libboost-all-dev libimlib2-dev
   # Additional Raspberry Pi specific packages may be needed

Build System Requirements
========================

**CMake** (version 3.07 or later)
**C++ Compiler** with C++17 support (GCC 7+, Clang 5+, MSVC 2017+)
**Make** or equivalent build system

Platform Detection
==================

The build system automatically detects the target platform and enables appropriate applications:

Raspberry Pi Detection
---------------------

The CMake configuration automatically detects Raspberry Pi systems by checking for the existence of ``/boot/LICENCE.broadcom``. When detected:

* Additional Raspberry Pi-specific applications are enabled
* Hardware-specific optimizations are applied
* Platform-specific dependencies are configured

.. code-block:: cmake

   if (EXISTS "/boot/LICENCE.broadcom")
       set(BUILD_RASPBERRYPI true)
       message("Build on Raspberry Pi is enabled")
   endif()

Enabled Applications by Platform
-------------------------------

**All Platforms:**
   * CubeTestApp - Basic 3D functionality demonstration
   * PixelFlow3 - Fluid dynamics simulation

**Raspberry Pi Only:**
   * ImuTest - IMU sensor integration
   * PixelFlow - Original fluid dynamics (with hardware optimizations)
   * PixelFlow2 - Enhanced fluid dynamics
   * Rainbow - Particle system with IMU integration

Building from Source
====================

Basic Build Process
------------------

1. **Clone or extract the source code**

2. **Create build directory:**

   .. code-block:: bash

      mkdir -p build
      cd build

3. **Configure with CMake:**

   .. code-block:: bash

      cmake ..

4. **Compile all applications:**

   .. code-block:: bash

      make

5. **Build specific application:**

   .. code-block:: bash

      make <application_name>
      # Example: make cubetestapp

Build Configuration Options
---------------------------

**Build Type Configuration**

The build system defaults to Release mode for optimized performance:

.. code-block:: bash

   # Explicit Release build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   
   # Debug build with symbols
   cmake -DCMAKE_BUILD_TYPE=Debug ..

**Custom Installation Prefix**

.. code-block:: bash

   cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..

**Platform-Specific Options**

For **macOS** systems, the build system automatically configures library paths:

.. code-block:: bash

   # Automatic configuration for Homebrew paths
   cmake -DIMLIB2_ROOT=/opt/homebrew/Cellar/imlib2/1.12.2 ..

Dependency Resolution
====================

CMake Package Discovery
----------------------

The build system uses CMake's ``find_package`` mechanism to locate dependencies:

.. code-block:: cmake

   find_package(Boost 1.58.0 REQUIRED COMPONENTS thread log system)

   find_package(matrixapplication 0.4 REQUIRED)

Library Linking Configuration
----------------------------

Each application links against the required libraries:

.. code-block:: cmake

   target_link_libraries(cubetestapp 
       matrixapplication::matrixapplication 
   )

Troubleshooting Build Issues
============================

Common Problems and Solutions
-----------------------------

**Missing matrixapplication Library**

.. code-block:: text

   Error: Could not find package configuration file provided by "matrixapplication"

*Solution*: Install the matrixserver framework (version >= 0.3) first. The build system
will automatically search ``./matrixserver/install/`` before falling back to system paths.
To use a local build of matrixserver, install it into that directory:

.. code-block:: bash

   # Inside the matrixserver repository
   cmake .. -DCMAKE_INSTALL_PREFIX=../install
   make && make install

Alternatively, for a custom location pass ``-DCMAKE_PREFIX_PATH=/your/path`` to CMake.

**Imlib2 Not Found**

.. code-block:: text

   Error: IMLIB2 Not Found

*Solution*: Install Imlib2 development packages or specify custom path:

.. code-block:: bash

   cmake -DIMLIB2_ROOT=/path/to/imlib2 ..

**Boost Version Conflicts**

.. code-block:: text

   Error: Boost version too old

*Solution*: Update Boost to version 1.58.0 or later, or specify custom Boost installation:

.. code-block:: bash

   cmake -DBOOST_ROOT=/path/to/boost ..

**C++17 Compiler Issues**

.. code-block:: text

   Error: C++17 features not available

*Solution*: Use a modern compiler that supports C++17:

.. code-block:: bash

   cmake -DCMAKE_CXX_COMPILER=g++-7 ..

Deployment
==========

Package Generation
-----------------

The project includes Debian package generation for easy deployment:

.. code-block:: bash

   # Generate .deb package
   make package

This creates a ``.deb`` file with:

* Dependency declaration on matrixserver package
* Proper installation paths (``/home/pi/APPS`` on Raspberry Pi)
* Package metadata and installation scripts

Manual Installation
------------------

For manual deployment:

1. **Copy executables** to target system
2. **Ensure matrixserver framework** is installed
3. **Install runtime dependencies** (Boost, Imlib2)
4. **Set executable permissions** if needed

Running Applications
===================

Server Requirement
------------------

All applications require a running matrixserver instance. Start the appropriate server for your hardware:

**Development/Testing:**

.. code-block:: bash

   # Software simulator
   /path/to/matrixserver/build/server_simulator

**Hardware Deployment:**

.. code-block:: bash

   # FPGA with USB interface
   /path/to/matrixserver/build/server_FPGA
   
   # Raspberry Pi SPI interface
   /path/to/matrixserver/build/server_FPGA_RPISPI
   
   # Direct GPIO control
   /path/to/matrixserver/build/server_RGBMatrix

Application Execution
--------------------

Once a server is running, launch any example application:

.. code-block:: bash

   # From build directory
   ./cubetestapp
   ./snake
   ./pixelflow3
   # etc.

The applications will automatically:

* Detect and connect to running matrixserver instances
* Configure appropriate frame rates and display settings
* Begin rendering to the connected display hardware

Connection Priority
------------------

Applications attempt connections in this order:

1. **TCP localhost:2017** (default)
2. **IPC message queue** (local fallback)
3. **Unix domain sockets** (alternative local)

This ensures robust connectivity across different deployment scenarios.