============
API Reference
============

This section provides detailed technical reference for the core framework classes and common programming patterns used throughout the LED Matrix Cube example applications.

.. toctree::
   :maxdepth: 2
   :caption: API Documentation:

   cubeapplication
   matrixapplication
   common-patterns
   input-handling
   color-system

Framework Base Classes
=====================

The LED Matrix Cube framework provides two primary base classes that applications inherit from, each designed for different display paradigms and use cases.

CubeApplication Class
--------------------

The ``CubeApplication`` class is the foundation for 3D volumetric applications that render content in three-dimensional LED cube displays.

:doc:`cubeapplication`
   Complete API reference for 3D cube applications including coordinate systems, rendering methods, and surface navigation.

MatrixApplication Class
----------------------

The ``MatrixApplication`` class provides the foundation for traditional 2D matrix display applications.

:doc:`matrixapplication`
   Complete API reference for 2D matrix applications including screen management and pixel control.

Common Patterns and Utilities
=============================

Programming Patterns
-------------------

:doc:`common-patterns`
   Standard programming patterns, application lifecycle management, and best practices used across all example applications.

Input and Control Systems
-------------------------

:doc:`input-handling`
   Comprehensive guide to joystick input, sensor integration, and user interaction patterns.

Color and Visual Systems
------------------------

:doc:`color-system`
   Color management, visual effects, and rendering optimization techniques.

Quick Reference
===============

Essential Classes
----------------

**CubeApplication**
   Base class for 3D volumetric applications

   * ``CubeApplication(int fps)`` - Constructor with frame rate
   * ``bool loop()`` - Main render loop (pure virtual)
   * ``void clear()`` - Clear 3D space
   * ``void render()`` - Send frame to display
   * ``void setPixel3D(Vector3i pos, Color color)`` - Set individual voxel
   * ``void drawLine3D(Vector3i start, Vector3i end, Color color)`` - 3D line drawing
   * ``void drawText(ScreenNumber screen, Vector2i pos, Color color, string text)`` - Text on cube faces

**MatrixApplication**  
   Base class for 2D matrix applications

   * ``MatrixApplication(int fps)`` - Constructor with frame rate
   * ``bool loop()`` - Main render loop (pure virtual)
   * ``void render()`` - Send frame to display
   * ``std::vector<Screen*> screens`` - Available display screens

Essential Data Types
-------------------

**Vector3i / Vector3f**
   3D coordinate vectors for integer and floating-point positions

**Vector2i / Vector2f**
   2D coordinate vectors for screen positioning

**Color**
   RGB color representation with utility methods

   * ``Color(uint8_t r, uint8_t g, uint8_t b)`` - RGB constructor
   * ``Color::red()`` / ``Color::blue()`` / ``Color::green()`` - Predefined colors
   * ``Color::white()`` / ``Color::black()`` - Grayscale colors
   * ``Color::random()`` / ``Color::randomBlue()`` - Random color generation

**ScreenNumber**
   Enumeration for cube face identification

   * ``ScreenNumber::front`` / ``ScreenNumber::back``
   * ``ScreenNumber::left`` / ``ScreenNumber::right``
   * ``ScreenNumber::top`` / ``ScreenNumber::bottom``

**Joystick**
   Input device interface

   * ``bool getButtonPress(int button)`` - Detect button presses
   * ``int getAxis(int axis)`` - Get axis values (-1, 0, 1)
   * ``void clearAllButtonPresses()`` - Reset button states

Application Lifecycle
====================

Standard Application Pattern
---------------------------

All applications follow this standard lifecycle pattern:

.. code-block:: cpp

   class MyApplication : public CubeApplication {
   public:
       MyApplication() : CubeApplication(fps) {
           // Initialize application resources
           // Set up joysticks, load data, etc.
       }
       
       bool loop() override {
           // 1. Input Processing
           handleInput();
           
           // 2. State Updates
           updatePhysics();
           updateAnimation();
           
           // 3. Rendering
           clear();                    // Clear previous frame
           drawContent();              // Render new content
           render();                   // Send to display
           
           return true;                // Continue running
       }
       
   private:
       void handleInput() { /* process joystick input */ }
       void updatePhysics() { /* update game state */ }
       void updateAnimation() { /* advance animations */ }
       void drawContent() { /* render to display buffer */ }
   };

Frame Rate Management
--------------------

Applications specify their target frame rate in the constructor:

.. code-block:: cpp

   // Common frame rates used in examples
   CubeApplication(20)    // 20 FPS - Minimal load (Blackout3D)
   CubeApplication(30)    // 30 FPS - Balanced (CubeTestApp)
   CubeApplication(40)    // 40 FPS - Smooth animation (Snake, PixelFlow)

The framework automatically handles timing regulation to maintain consistent frame rates.

Connection Architecture
======================

Server Communication
-------------------

Applications automatically connect to matrixserver instances through multiple transport mechanisms:

**Connection Priority**:

1. TCP Network (localhost:2017)
2. IPC Message Queues  
3. Unix Domain Sockets

**Automatic Fallback**: Applications try each connection method in sequence until successful.

**Server Types**:

* ``server_simulator`` - Software renderer for development
* ``server_FPGA`` - Hardware interface for FPGA systems
* ``server_FPGA_RPISPI`` - Raspberry Pi SPI interface
* ``server_RGBMatrix`` - Direct GPIO control

Performance Considerations
=========================

Optimization Techniques
----------------------

**Static Variables**: Use static variables in the main loop for persistent state without class member overhead.

**Oversampling**: Some applications use oversampling for smooth physics:

.. code-block:: cpp

   for (int sample = 0; sample < OVERSAMPLING; sample++) {
       updatePhysics();
   }

**Selective Updates**: Skip expensive operations when possible:

.. code-block:: cpp

   if (counter % 2 == 0) {
       updateExpensiveCalculation();
   }

**Memory Management**: Use STL algorithms for efficient container management:

.. code-block:: cpp

   // Remove expired particles
   particles.erase(std::remove_if(particles.begin(), particles.end(),
       [](const Particle& p) { return p.shouldDelete(); }),
       particles.end());

Error Handling
=============

Connection Failures
------------------

Applications should handle connection failures gracefully by attempting multiple connection methods and providing useful feedback.

Resource Management
------------------

Follow RAII principles for automatic resource cleanup and exception safety.

This API reference provides the foundation for developing custom applications using the LED Matrix Cube framework. For complete implementation examples, refer to the individual application documentation.