============
Applications
============

This section provides detailed documentation for each example application included in the LED Matrix Cube project. The applications are organized by their primary functionality and inheritance from the framework base classes.

.. toctree::
   :maxdepth: 2
   :caption: Application Categories:

   3d-cube-applications
   2d-matrix-applications  
   animation-effects
   hardware-integration

Application Overview
===================

The example applications demonstrate various capabilities of the matrixserver framework:

3D Cube Applications
-------------------

These applications inherit from ``CubeApplication`` and render 3D volumetric content:

.. toctree::
   :maxdepth: 1

   cubetestapp
   snake
   blackout3d
   breakout3d

**CubeTestApp** - :doc:`cubetestapp`
   Basic 3D functionality demonstration with animated lines and screen text

**Snake** - :doc:`snake`
   3D implementation of Snake game with multi-player support and joystick control

**Blackout3D** - :doc:`blackout3d`
   Minimal interactive 3D application template for development

**Breakout3D** - :doc:`breakout3d`
   Full-featured 3D Breakout game with physics and multiplayer

2D Matrix Applications
---------------------

These applications inherit from ``MatrixApplication`` for traditional 2D displays:

.. toctree::
   :maxdepth: 1

   genetic
   picture

**Genetic** - :doc:`genetic`
   Genetic algorithm visualization with evolving color patterns

**Picture** - :doc:`picture`
   Image display functionality with automatic reloading

Animation and Effects Applications
---------------------------------

Specialized applications focused on visual effects and animations:

.. toctree::
   :maxdepth: 1

   pixelflow
   rainbow

**PixelFlow Series** - :doc:`pixelflow`
   Advanced particle simulation applications with three variants:
   
   * **PixelFlow** - IMU-integrated fluid simulation (Raspberry Pi, MPU6050 sensor)
   * **PixelFlow2** - Enhanced IMU-based system with optimized particle management  
   * **PixelFlow3** - Cross-platform particle system with joystick controls

**Rainbow** - :doc:`rainbow`
   Particle system with IMU integration and multiple display modes


Hardware Integration
-------------------

Applications demonstrating sensor and hardware integration:

.. toctree::
   :maxdepth: 1

   imutest

**ImuTest** - :doc:`imutest`
   IMU sensor integration for Raspberry Pi platforms

Application Selection Guide
===========================

Choose applications based on your display hardware and requirements:

For 3D LED Cube Hardware
------------------------

**Getting Started:**
   Start with :doc:`cubetestapp` to verify 3D functionality and cube orientation

**Interactive Games:**
   Try :doc:`snake` for multi-player gaming or :doc:`breakout3d` for single-player arcade action

**Development Template:**
   Use :doc:`blackout3d` as a minimal starting point for custom 3D applications

For 2D Matrix Displays
----------------------

**Visual Algorithms:**
   Use :doc:`genetic` to demonstrate algorithmic visualization

**Content Display:**
   Use :doc:`picture` for displaying images and graphics

For Effects and Animation
------------------------

**Particle Systems:**
   Try :doc:`pixelflow` series for fluid dynamics or :doc:`rainbow` for IMU-responsive effects


Platform Compatibility
======================

Application availability varies by platform:

All Platforms (Desktop + Raspberry Pi)
--------------------------------------

* :doc:`cubetestapp` - Always available for testing
* :doc:`pixelflow` (PixelFlow3 variant) - Cross-platform particle effects

Raspberry Pi Only
-----------------

* :doc:`imutest` - Requires IMU hardware integration
* :doc:`pixelflow` - Original PixelFlow and PixelFlow2 variants require MPU6050 IMU sensor
* :doc:`rainbow` - IMU-integrated particle system

Build Configuration
-------------------

Most applications are available by default. Some applications may be commented out in ``CMakeLists.txt`` for specific deployment configurations. To enable all applications, ensure the corresponding ``add_subdirectory()`` lines are uncommented in the main ``CMakeLists.txt``:

.. code-block:: cmake

   # Example CMakeLists.txt entries
   add_subdirectory(Genetic)
   add_subdirectory(Snake)
   add_subdirectory(Blackout3D)
   add_subdirectory(Breakout3D)
   add_subdirectory(Picture)

Common Application Features
===========================

Input Handling
--------------

Most interactive applications support:

* **USB Joystick Control**: Standard gamepad input
* **Multi-Controller Support**: Up to 4 simultaneous controllers
* **Button Mapping**: Consistent control schemes across applications

Frame Rate Management
--------------------

Applications specify target frame rates:

* **20 FPS**: Blackout3D (minimal load)
* **30 FPS**: CubeTestApp (balanced performance) 
* **40 FPS**: Snake, PixelFlow series (smooth animation)

Performance Tuning
------------------

Applications include various performance optimizations:

* **Adaptive Frame Rates**: Automatic adjustment under load
* **Selective Updates**: Skip expensive operations when possible
* **Memory Management**: Efficient particle and object management

Development Patterns
====================

All applications follow consistent development patterns:

Application Structure
--------------------

.. code-block:: cpp

   class ApplicationName : public BaseApplicationClass {
   public:
       ApplicationName();                    // Constructor with FPS
       virtual bool loop() override;         // Main render loop
   
   private:
       // Application-specific members
   };

Main Loop Pattern
----------------

.. code-block:: cpp

   bool loop() override {
       // 1. Input processing
       handleInput();
       
       // 2. State updates  
       updatePhysics();
       
       // 3. Rendering
       clear();
       drawContent();
       render();
       
       return true;  // Continue running
   }

This consistent structure makes it easy to understand and modify any application in the collection.