====================================
LED Matrix Cube Examples
====================================

Welcome to the LED Matrix Cube Examples documentation. This comprehensive guide covers example applications for the matrixserver framework, designed to demonstrate various capabilities of 3D LED cube displays and 2D matrix panels.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   overview
   installation
   cmake
   architecture
   parameters
   applications/index
   api/index

Overview
========

The LED Matrix Cube example applications showcase the capabilities of the matrixserver framework through various interactive demonstrations, games, and visualizations. These examples range from simple 3D graphics tests to complex interactive applications with joystick input and sensor integration.

Key Features
------------

* **3D Volumetric Rendering**: Full 3D pixel manipulation in a cubic LED matrix
* **2D Matrix Support**: Traditional flat LED panel applications  
* **Interactive Input**: Joystick and sensor integration
* **Cross-Platform**: Runs on desktop systems and Raspberry Pi
* **Modular Architecture**: Clean separation between application logic and hardware interface
* **Real-Time Performance**: Optimized for smooth animations and responsive interactions

Quick Start
===========

To get started with the LED Matrix Cube examples:

1. **Prerequisites**: Install the matrixserver framework and dependencies
2. **Build**: Use CMake to compile the applications
3. **Run**: Start a matrixserver instance and launch an example application

.. code-block:: bash

   # Build all applications
   mkdir -p build && cd build
   cmake .. && make
   
   # Start the simulator server (in another terminal)
   /path/to/matrixserver/build/server_simulator
   
   # Run an example application
   ./cubetestapp

Application Categories
=====================

The examples are organized into several categories:

**3D Cube Applications**
   Applications that inherit from ``CubeApplication`` and render 3D volumetric content:
   
   * :doc:`applications/cubetestapp` - Basic 3D functionality demonstration and testing
   * :doc:`applications/snake` - Multi-player 3D Snake game with AI opponents
   * :doc:`applications/blackout3d` - Minimal interactive 3D application template
   * :doc:`applications/breakout3d` - Full-featured 3D Breakout game with physics

**2D Matrix Applications**
   Applications that inherit from ``MatrixApplication`` for traditional 2D display:
   
   * :doc:`applications/genetic` - Genetic algorithm color evolution visualization
   * :doc:`applications/picture` - Image display with animation support and file watching

**Animation and Effects**
   Specialized applications for visual effects:
   
   * :doc:`applications/pixelflow` - Advanced particle simulation series with IMU integration and cross-platform variants
   * :doc:`applications/rainbow` - Particle system with IMU integration (Raspberry Pi only)

**Hardware Integration**
   Applications demonstrating sensor and input integration:
   
   * :doc:`applications/imutest` - IMU sensor integration (Raspberry Pi only)

Framework Integration
====================

All example applications integrate with the matrixserver framework through:

* **Connection Management**: Automatic connection to running matrixserver instances via TCP, IPC, or Unix sockets
* **Frame Rate Control**: Configurable FPS with automatic timing regulation
* **Hardware Abstraction**: Universal API that works with simulator, FPGA, and GPIO-based displays
* **Input Processing**: Standardized joystick and sensor input handling

Documentation Structure
=======================

This documentation is organized to help you understand both the individual applications and the underlying framework:

* **Installation Guide**: Complete setup instructions for all platforms
* **CMake Reference**: Detailed build system configuration
* **Architecture Overview**: Framework design and application patterns  
* **Application Guides**: In-depth documentation for each example
* **API Reference**: Technical reference for base classes and common patterns

Getting Help
============

* Check the :doc:`installation` guide for setup issues
* Review the :doc:`architecture` documentation to understand framework concepts
* Examine individual :doc:`applications/index` for specific usage examples
* Refer to the :doc:`api/index` for technical implementation details

License and Contributing
========================

This project is part of the matrixserver framework ecosystem. Please refer to the main project repository for licensing information and contribution guidelines.

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`