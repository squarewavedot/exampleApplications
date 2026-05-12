========
Overview
========

The LED Matrix Cube Examples project provides a comprehensive collection of demonstration applications for the matrixserver framework. These examples showcase the capabilities of 3D LED cube displays and 2D matrix panels through interactive games, visualizations, and effects.

Project Purpose
===============

The example applications serve multiple purposes:

**Educational**: Demonstrate proper usage patterns and best practices for the matrixserver framework
**Functional**: Provide ready-to-use applications for LED matrix displays
**Reference**: Serve as starting points for custom application development
**Testing**: Validate framework functionality across different platforms and hardware configurations

Architecture Overview
=====================

The project follows a layered architecture that separates application logic from hardware interfacing:

Framework Layer
---------------

The matrixserver framework provides the core infrastructure:

* **Connection Management**: Handles communication with various server implementations
* **Hardware Abstraction**: Unified API for different display technologies
* **Timing Control**: Frame rate regulation and animation timing
* **Input Processing**: Standardized joystick and sensor input handling

Application Layer
-----------------

Example applications build on two main base classes:

**CubeApplication**
   For 3D volumetric applications that render into a cubic LED matrix:
   
   * 3D coordinate system with (x, y, z) addressing
   * Volumetric pixel manipulation
   * 3D line drawing and text rendering on cube faces
   * Edge detection and surface navigation

**MatrixApplication**
   For traditional 2D matrix displays:
   
   * 2D coordinate system with (x, y) addressing
   * Pixel-level control of flat LED panels
   * Screen-based rendering for multiple panel configurations

Hardware Support
================

The examples are designed to work across multiple hardware platforms:

Development Platforms
--------------------

* **Desktop Systems**: macOS, Linux, Windows with software simulator
* **Cross-compilation**: Support for embedded targets

Production Platforms
--------------------

* **Raspberry Pi**: GPIO-based matrix panels, SPI interfaces
* **FPGA Systems**: Custom hardware with FTDI USB interfaces
* **Dedicated Controllers**: Purpose-built LED matrix controllers

Display Technologies
====================

The framework and examples support various LED matrix configurations:

3D Cube Displays
----------------

* **Volumetric Matrices**: True 3D pixel arrays forming cubic displays
* **Multi-sided Cubes**: Six separate 2D panels arranged in cube formation
* **Custom Geometries**: Adaptable to non-cubic 3D arrangements

2D Matrix Displays
------------------

* **Single Panels**: Individual LED matrix displays
* **Tiled Arrays**: Multiple panels arranged in larger displays
* **Various Resolutions**: From small 8x8 matrices to large 64x64+ displays

Connection Architecture
======================

The examples connect to matrixserver instances through multiple transport mechanisms:

Primary Connection Methods
-------------------------

**TCP Network**
   * Default connection method
   * Supports remote server instances
   * Cross-platform compatibility
   * Default port: 2017

**IPC (Inter-Process Communication)**
   * High-performance local communication
   * Uses Boost message queues
   * Ideal for same-machine deployments

**Unix Domain Sockets**
   * Low-latency local communication
   * POSIX-compliant systems
   * Efficient for co-located processes

Server Implementations
---------------------

The examples work with various matrixserver implementations:

* ``server_simulator``: Software renderer for development
* ``server_FPGA``: FTDI USB interface for FPGA systems
* ``server_FPGA_RPISPI``: Raspberry Pi SPI interface
* ``server_RGBMatrix``: Direct Raspberry Pi GPIO control

Platform-Specific Features
==========================

The build system automatically detects platform capabilities:

Raspberry Pi Detection
---------------------

The CMake configuration detects Raspberry Pi systems by checking for ``/boot/LICENCE.broadcom`` and enables additional applications:

* **IMU Integration**: MPU6050 sensor support
* **Hardware PWM**: Direct GPIO control
* **Specialized Applications**: Rainbow particle effects, PixelFlow variants

Cross-Platform Components
------------------------

All platforms support:

* **Core Applications**: CubeTestApp, PixelFlow3
* **Standard Input**: USB joystick support
* **Network Connectivity**: TCP-based server communication
* **Image Processing**: Imlib2-based picture display

Development Workflow
====================

The typical development and deployment workflow:

1. **Setup**: Install matrixserver framework and dependencies
2. **Development**: Use software simulator for application development
3. **Testing**: Validate applications with various server implementations
4. **Deployment**: Package applications for target hardware platforms

Package Management
==================

The project includes Debian package generation:

* **Dependency Management**: Automatic matrixserver dependency declaration
* **Installation Paths**: Standardized deployment locations (``/home/pi/APPS`` on Raspberry Pi)
* **System Integration**: Proper package metadata and installation scripts

This architecture provides flexibility for both development and production deployments while maintaining consistent APIs and behavior across different hardware platforms.