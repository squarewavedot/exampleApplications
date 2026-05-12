==========
Blackout3D
==========

**Blackout3D** is a minimal 3D application template that serves as the perfect starting point for developing custom LED cube applications. It demonstrates the essential structure and methods required for 3D cube rendering.

Overview
========

Blackout3D provides a clean, minimal template that:

* Inherits from ``CubeApplication`` for 3D volumetric rendering
* Implements the basic application loop pattern
* Connects to matrixserver at 20 FPS for minimal load
* Serves as a foundation for custom 3D applications

This application is ideal for:

* **Learning** the basic cube application structure
* **Starting** new 3D application development  
* **Testing** basic connectivity and rendering setup
* **Understanding** the minimal requirements for cube applications

Implementation
==============

Core Structure
--------------

.. code-block:: cpp

   // Blackout3D.h
   #include "CubeApplication.h"
   
   class Blackout3D : public CubeApplication {
   public:
       Blackout3D();
       virtual bool loop() override;
   };

Application Loop
----------------

The implementation demonstrates the minimal application loop:

.. code-block:: cpp

   // Blackout3D.cpp
   #include "Blackout3D.h"

   Blackout3D::Blackout3D() : CubeApplication(20, "cube10.local") {
       // Constructor: 20 FPS, connect to specific host
       // Add initialization code here
   }

   bool Blackout3D::loop() {
       clear();                    // Clear all voxels from previous frame
       
       // Add your custom 3D rendering logic here:
       // setPixel3D(Vector3i(x, y, z), Color::red());
       // drawLine3D(start, end, Color::blue());
       // drawText(front, pos, Color::white(), "Hello");
       
       render();                   // Send frame to server
       return true;                // Continue running (false to exit)
   }

Configuration
=============

Frame Rate
----------

Blackout3D runs at **20 FPS** to minimize system load while providing a responsive development environment:

.. code-block:: cpp

   Blackout3D() : CubeApplication(20, "cube10.local")

Connection
----------

By default connects to ``cube10.local``. Modify the host parameter to connect to different server instances:

.. code-block:: cpp

   // Local development
   Blackout3D() : CubeApplication(20, "localhost")
   
   // Specific IP address  
   Blackout3D() : CubeApplication(20, "192.168.1.100")

Development Guide
=================

Starting a New Application
--------------------------

1. **Copy the Blackout3D files** as a template:

   .. code-block:: bash
   
      cp -r Blackout3D/ MyNewApp/
      # Rename files and classes appropriately

2. **Modify the constructor** for your needs:

   .. code-block:: cpp
   
      MyNewApp() : CubeApplication(30, "localhost") {
          // Initialize your application state
          initializeGameObjects();
          loadResources();
      }

3. **Implement your rendering logic** in the loop method:

   .. code-block:: cpp
   
      bool MyNewApp::loop() {
          clear();
          
          // Update application state
          updatePhysics();
          handleInput();
          
          // Render 3D content
          drawGameObjects();
          drawUI();
          
          render();
          return true;
      }

Common Extensions
-----------------

**Adding Joystick Input:**

.. code-block:: cpp

   class MyNewApp : public CubeApplication {
   private:
       std::vector<Joystick*> joysticks;
       
   public:
       MyNewApp() : CubeApplication(30, "localhost") {
           joysticks.push_back(new Joystick(0));  // Player 1
       }
       
       bool loop() override {
           clear();
           
           // Handle input
           for(auto joystick : joysticks) {
               if(joystick->getButtonPress(0)) {
                   // Handle button press
               }
               joystick->clearAllButtonPresses();
           }
           
           render();
           return true;
       }
   };

**Adding 3D Objects:**

.. code-block:: cpp

   bool loop() override {
       clear();
       
       // Draw a 3D line
       drawLine3D(Vector3i(0, 0, 0), Vector3i(63, 63, 63), Color::red());
       
       // Set individual voxels
       setPixel3D(Vector3i(32, 32, 32), Color::blue());
       
       // Draw text on cube faces
       drawText(ScreenNumber::front, Vector2i(10, 10), 
               Color::white(), "Hello Cube!");
       
       render();
       return true;
   }

**Adding Animation:**

.. code-block:: cpp

   bool loop() override {
       static int counter = 0;
       static float time = 0.0f;
       
       clear();
       
       // Animated rotation
       time += 0.1f;
       int x = 32 + 20 * cos(time);
       int y = 32 + 20 * sin(time);
       int z = 32;
       
       setPixel3D(Vector3i(x, y, z), Color::red());
       
       render();
       counter++;
       return true;
   }

Building and Running
====================

Build Process
-------------

.. code-block:: bash

   # Build Blackout3D specifically
   mkdir -p build && cd build
   cmake .. && make blackout3d
   
   # Run the application
   ./blackout3d

Prerequisites
-------------

1. **MatrixServer Running**: Start appropriate server before launching:

   .. code-block:: bash
   
      # For development
      /path/to/matrixserver/build/server_simulator
      
      # For hardware
      /path/to/matrixserver/build/server_FPGA

2. **Dependencies Installed**: Ensure matrixapplication library is available

Troubleshooting
===============

Connection Issues
-----------------

* **Server not running**: Start matrixserver before launching application
* **Wrong host**: Verify the hostname/IP in the constructor
* **Port conflicts**: Check that port 2017 is available

Performance Issues
------------------

* **High CPU usage**: Blackout3D runs at low 20 FPS by design
* **Slow rendering**: Check server implementation performance
* **Memory leaks**: Ensure proper cleanup in destructor if adding dynamic objects

Next Steps
==========

After mastering Blackout3D, explore more complex applications:

* **CubeTestApp** - See comprehensive 3D rendering examples
* **Snake** - Learn game state management and input handling  
* **Breakout3D** - Study physics and collision detection
* **PixelFlow3** - Examine particle system implementation

The minimal nature of Blackout3D makes it the ideal foundation for understanding cube application development and serves as a clean starting point for any custom 3D LED cube project.