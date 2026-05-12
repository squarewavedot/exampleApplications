===========
CubeTestApp
===========

CubeTestApp is the foundational 3D demonstration application that showcases basic functionality of the LED Matrix Cube framework. It serves as both a functional test and a reference implementation for 3D cube applications.

Purpose and Features
===================

The CubeTestApp demonstrates core 3D rendering capabilities:

* **3D Line Drawing**: Animated lines moving through 3D space
* **Screen Text Rendering**: Text display on all six cube faces
* **Basic Animation**: Frame-based animation with loop counters
* **Framework Integration**: Proper use of CubeApplication base class

Technical Overview
=================

Class Structure
--------------

CubeTestApp inherits from ``CubeApplication`` and implements the basic application lifecycle:

.. code-block:: cpp

   class CubeTest : public CubeApplication {
   public:
       CubeTest();                    // Constructor with 30 FPS
       bool loop() override;          // Main render loop
   };

Frame Rate
---------

The application runs at **30 FPS**, providing smooth animation while maintaining reasonable performance:

.. code-block:: cpp

   CubeTest::CubeTest() : CubeApplication(30) {
       // 30 FPS initialization
   }

Core Functionality
=================

3D Line Animation
----------------

The application renders two animated 3D lines that move through the cube space:

.. code-block:: cpp

   // Red line moving vertically
   drawLine3D(Vector3i(0,0,CUBESIZE-loopcount%CUBESIZE),
              Vector3i(CUBESIZE,0,CUBESIZE-loopcount%CUBESIZE), 
              Color::red());
   
   // Blue line moving horizontally  
   drawLine3D(Vector3i(loopcount%CUBESIZE,0,CUBESIZE),
              Vector3i(loopcount%CUBESIZE,0,0), 
              Color::blue());

**Animation Details:**

* Lines use the loop counter modulo cube size for cyclic movement
* Red line moves along the Z-axis (vertical)
* Blue line moves along the X-axis (horizontal)
* Both lines maintain constant Y position (front face)

Screen Text Display
------------------

The application displays identification text on all six cube faces:

.. code-block:: cpp

   drawText(ScreenNumber::front,  Vector2i(centered, centered), Color::white(), "Screen 0 front");
   drawText(ScreenNumber::right,  Vector2i(centered, centered), Color::white(), "Screen 1 right");
   drawText(ScreenNumber::back,   Vector2i(centered, centered), Color::white(), "Screen 2 back");
   drawText(ScreenNumber::left,   Vector2i(centered, centered), Color::white(), "Screen 3 left");
   drawText(ScreenNumber::top,    Vector2i(centered, centered), Color::white(), "Screen 4 top");
   drawText(ScreenNumber::bottom, Vector2i(centered, centered), Color::white(), "Screen 5 bottom");

**Text Features:**

* **Screen Identification**: Each face shows its screen number and name
* **Centered Positioning**: Uses ``CharacterBitmaps::centered`` for automatic centering
* **Consistent Styling**: White text on all faces for visibility
* **Orientation Verification**: Helps verify correct cube face mapping

Main Loop Implementation
=======================

The main loop follows the standard CubeApplication pattern:

.. code-block:: cpp

   bool CubeTest::loop() {
       static int loopcount = 0;
       
       clear();                        // Clear previous frame
       
       // Render 3D lines
       drawLine3D(/* animated red line */);
       drawLine3D(/* animated blue line */);
       
       // Render screen text
       drawText(/* all six faces */);
       
       render();                       // Send frame to display
       loopcount++;                    // Increment animation counter
       return true;                    // Continue running
   }

**Loop Characteristics:**

* **Static Counter**: Maintains animation state across loop iterations
* **Clear-Render Cycle**: Standard framework pattern for frame updates
* **Continuous Animation**: Loop counter drives animation timing
* **Always Returns True**: Application runs indefinitely

Development and Testing Use
==========================

Verification Tool
----------------

CubeTestApp serves as a comprehensive verification tool:

**3D Functionality**:
   * Verifies 3D coordinate system orientation
   * Tests line drawing algorithms
   * Confirms proper Z-buffer handling

**Display Mapping**:
   * Validates screen number assignments
   * Verifies text rendering on each face
   * Confirms proper face orientation

**Framework Integration**:
   * Tests connection to matrixserver
   * Validates frame rate control
   * Confirms proper application lifecycle

Reference Implementation
-----------------------

The application demonstrates best practices:

**Clean Structure**:
   * Proper inheritance from CubeApplication
   * Standard constructor and loop implementation
   * Appropriate use of framework APIs

**Performance Patterns**:
   * Efficient static variable usage
   * Minimal computation in main loop
   * Proper resource management

**Code Organization**:
   * Clear separation of concerns
   * Readable animation logic
   * Consistent coding style

Build and Execution
==================

Compilation
----------

CubeTestApp is always enabled in the build configuration:

.. code-block:: bash

   # Build from source directory
   mkdir -p build && cd build
   cmake ..
   make cubetestapp

Execution
--------

Run the application with a matrixserver instance:

.. code-block:: bash

   # Start server (in separate terminal)
   /path/to/matrixserver/build/server_simulator
   
   # Run application
   ./cubetestapp

**Expected Behavior**:

* Two animated lines moving through 3D space
* Text labels on all six cube faces
* Smooth 30 FPS animation
* Continuous operation until interrupted

Troubleshooting
==============

Common Issues
------------

**No Animation Visible**:
   * Verify matrixserver is running
   * Check server connection (default TCP localhost:2017)
   * Confirm display hardware is properly configured

**Incorrect Face Mapping**:
   * Verify cube orientation matches expected layout
   * Check server configuration for face assignments
   * Confirm hardware wiring matches framework expectations

**Performance Issues**:
   * Monitor CPU usage during execution
   * Verify frame rate is maintaining 30 FPS
   * Check for system resource constraints

Customization
============

Modification Examples
-------------------

**Change Animation Speed**:

.. code-block:: cpp

   // Faster animation (every 2nd frame)
   drawLine3D(Vector3i(0,0,CUBESIZE-(loopcount/2)%CUBESIZE), ...);
   
   // Slower animation (every 4th frame)  
   drawLine3D(Vector3i(0,0,CUBESIZE-(loopcount*4)%CUBESIZE), ...);

**Add More Lines**:

.. code-block:: cpp

   // Additional diagonal line
   drawLine3D(Vector3i(loopcount%CUBESIZE, loopcount%CUBESIZE, 0),
              Vector3i(loopcount%CUBESIZE, loopcount%CUBESIZE, CUBESIZE),
              Color::green());

**Custom Screen Text**:

.. code-block:: cpp

   // Custom messages
   drawText(ScreenNumber::front, Vector2i(centered, centered), 
            Color::cyan(), "Hello Cube!");

This application provides an excellent starting point for understanding 3D cube development and serves as a reliable test for verifying system functionality.