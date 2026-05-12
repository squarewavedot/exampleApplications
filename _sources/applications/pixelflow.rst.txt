=========
PixelFlow
=========

**PixelFlow** is a series of advanced particle simulation applications that demonstrate fluid dynamics, physics-based particle systems, and sensor integration. The PixelFlow family consists of three variants, each showcasing different levels of complexity and platform integration.

Overview
========

The PixelFlow series includes:

* **PixelFlow** - IMU-integrated fluid simulation for Raspberry Pi with hardware sensor input
* **PixelFlow2** - Enhanced IMU-based particle system with improved physics and lifetime management  
* **PixelFlow3** - Cross-platform particle system with joystick controls (no IMU dependency)

All variants feature:

* **3D Particle Physics** - Realistic particle movement with velocity and acceleration
* **Surface Dynamics** - Particles that flow along cube surfaces with edge transitions
* **Dynamic Color Systems** - Automatic color cycling with randomized variations
* **Fade Effects** - Configurable particle persistence and trail rendering
* **Efficient Memory Management** - Smart pointer-based particle lifecycle

Application Variants
====================

PixelFlow (Original) - IMU Integration
--------------------------------------

**Platform**: Raspberry Pi only (requires MPU6050 IMU sensor)

**Key Features**:
* **IMU Sensor Integration** - Uses MPU6050 accelerometer for motion-responsive particles
* **Surface-Aware Spawning** - Particles spawn on cube faces based on IMU orientation
* **Gravity Simulation** - Real-world gravity affects particle movement via IMU input
* **High Particle Density** - Up to 60 particles per frame for dense fluid effects

.. code-block:: cpp

   class PixelFlow : public CubeApplication {
   private:
       Mpu6050 Imu;                    // MPU6050 IMU sensor integration
       
   public:
       PixelFlow() : CubeApplication(40) {}
       
       bool loop() override {
           fade(0.85);                 // Particle persistence
           
           // Spawn 60 particles per frame
           for (int i = 0; i < 60; i++) {
               // Determine spawn surface from IMU
               auto imuPoint = Imu.getCubeAccIntersect();
               
               // Set initial velocity based on surface orientation
               Vector3f startSpeed = calculateSurfaceVelocity(imuPoint);
               
               rdrops.push_back(std::make_shared<Drop>(
                   maxPos, imuPoint.cast<float>(), startSpeed, 
                   Vector3f(0,0,0), currentColor));
           }
           
           // Apply IMU-based acceleration to existing particles
           for(auto drop : rdrops) {
               drop->acceleration(Imu.getAcceleration() * 
                   (-0.1f + random_variation));
               drop->step();
               setPixel3D(drop->iPosition(), drop->color());
           }
           
           render();
           return true;
       }
   };

PixelFlow2 - Enhanced IMU System
-------------------------------

**Platform**: Raspberry Pi only (requires MPU6050 IMU sensor)

**Key Features**:
* **Improved Particle Lifetime** - Optimized step count management (180 steps vs 260)
* **Enhanced Physics Model** - Refined acceleration and velocity calculations  
* **Better Surface Dynamics** - Improved particle flow along cube surfaces
* **Optimized Performance** - More efficient particle spawning and management

.. code-block:: cpp

   class PixelFlow2 : public CubeApplication {
   private:
       class Drop : public SurfaceParticle {
           int stepCount;                // Particle lifetime counter
           bool rdyDelete_;              // Deletion flag
           
       public:
           void step() override {
               SurfaceParticle::step();  // Advanced surface physics
               
               if (stepCount > 180) {    // Optimized lifetime
                   rdyDelete_ = true;
               }
               stepCount++;
           }
       };
   };

PixelFlow3 - Cross-Platform Interactive
---------------------------------------

**Platform**: All platforms (desktop and Raspberry Pi)

**Key Features**:
* **Joystick Integration** - Interactive controls without IMU dependency
* **Cross-Platform Compatibility** - Runs on any system with matrixserver
* **Simplified Physics** - No IMU dependency, uses programmed particle behavior
* **Interactive Controls** - Real-time parameter adjustment via gamepad

.. code-block:: cpp

   class PixelFlow : public CubeApplication {
   private:
       std::vector<Joystick*> joysticks;
       
   public:
       PixelFlow() : CubeApplication(40) {
           // Initialize up to 4 joysticks for interaction
           for(int i = 0; i < 4; i++) {
               joysticks.push_back(new Joystick(i));
           }
       }
       
       bool loop() override {
           static bool isPaused = false;
           
           // Process joystick input
           for (auto joystick : joysticks) {
               if (joystick->getButtonPress(0)) {
                   counterColChange++;    // Change color scheme
               }
               if (joystick->getButtonPress(3)) {
                   isPaused = !isPaused; // Toggle pause
               }
               joystick->clearAllButtonPresses();
           }
           
           if (isPaused) return true;
           
           fade(0.85);
           
           // Create 30 particles with random motion
           for (int i = 0; i < 30; i++) {
               float randAngle = rand() % 360;
               float vx = 0.5 * cos(randAngle * PI / 180);
               float vy = 0.5 * sin(randAngle * PI / 180);
               
               rdrops.push_back(std::make_shared<Drop>(
                   maxPos, centerPosition, Vector3f(vx, vy, 0),
                   Vector3f(0,0,0), currentColor));
           }
           
           // Update particles
           for(auto drop : rdrops) {
               drop->step();
               setPixel3D(drop->iPosition(), drop->color());
           }
           
           cleanupParticles();
           render();
           return true;
       }
   };

Particle System Architecture
============================

All PixelFlow variants share a sophisticated particle system hierarchy:

Particle Base Class
-------------------

The foundation class providing basic physics:

.. code-block:: cpp

   class PixelFlow::Particle {
   protected:
       Vector3f position_;             // Current position
       Vector3f velocity_;             // Current velocity  
       Vector3f acceleration_;         // Current acceleration
       Color color_;                   // Particle color
       
   public:
       virtual void step() {
           accelerate();               // Apply acceleration
           move();                     // Update position
       }
       
       void move() {
           position_ += velocity_;     // Basic Euler integration
       }
       
       void accelerate() {
           velocity_ += acceleration_; // Update velocity
       }
       
       Vector3i iPosition() {
           return Vector3i(round(position_[0]), 
                          round(position_[1]), 
                          round(position_[2]));
       }
   };

SurfaceParticle Class
--------------------

Specialized particle that flows along cube surfaces:

.. code-block:: cpp

   class PixelFlow::SurfaceParticle : public Particle {
   protected:
       Vector3i maxPosition;           // Cube boundaries
       EdgeNumber lastEdge;            // Last cube edge
       Vector3i lastIPosition;         // Previous position
       
   public:
       void step() override {
           accelerateOnSurface();      // Surface-aware acceleration
           move();                     // Update position
           warp();                     // Handle edge transitions
       }
       
       void accelerateOnSurface() {
           // Apply acceleration based on current surface
           switch(getScreenNumber(iPosition())) {
               case top: case bottom:
                   velocity_[0] += acceleration_[0];
                   velocity_[1] += acceleration_[1];
                   break;
               case front: case back:
                   velocity_[0] += acceleration_[0];
                   velocity_[2] += acceleration_[2];
                   break;
               case left: case right:
                   velocity_[1] += acceleration_[1];
                   velocity_[2] += acceleration_[2];
                   break;
           }
       }
       
       void warp() {
           // Handle edge transitions and velocity mapping
           Vector3i currentPosition = iPosition();
           EdgeNumber currentEdge = getEdgeNumber(currentPosition);
           
           if(currentEdge != anyEdge && currentEdge != lastEdge) {
               // Map velocity components across edges
               switch (currentEdge) {
                   case topLeft: case topRight: 
                   case bottomRight: case bottomLeft:
                       std::swap(velocity_[2], velocity_[0]);
                       break;
                   case topFront: case topBack:
                   case bottomBack: case bottomFront:
                       std::swap(velocity_[2], velocity_[1]);
                       break;
                   case frontRight: case backLeft:
                   case leftFront: case rightBack:
                       std::swap(velocity_[0], velocity_[1]);
                       break;
               }
               
               // Reflect at boundaries if needed
               handleBoundaryReflection(currentPosition);
           }
           
           lastEdge = currentEdge;
           lastIPosition = currentPosition;
       }
   };

Drop Class Implementation
------------------------

The final particle implementation with lifecycle management:

.. code-block:: cpp

   class PixelFlow::Drop : public SurfaceParticle {
   private:
       float vxOld_, vyOld_;           // Stored velocities for surface sliding
       int stepCount;                 // Lifetime counter
       bool rdyDelete_;               // Cleanup flag
       
   public:
       Drop(Vector3i maxPos, Vector3f pos, Vector3f vel, 
            Vector3f accel, Color col)
           : SurfaceParticle(maxPos, pos, vel, accel, col) {
           stepCount = 0;
           rdyDelete_ = false;
           vxOld_ = vyOld_ = 0.0f;
       }
       
       void step() override {
           SurfaceParticle::step();    // Advanced surface physics
           
           // Lifetime management varies by variant:
           // PixelFlow: 260 steps
           // PixelFlow2: 180 steps  
           // PixelFlow3: Similar to MatrixRain
           if (stepCount > getMaxLifetime()) {
               rdyDelete_ = true;
           }
           stepCount++;
       }
       
       bool getRdyDelete() { return rdyDelete_; }
   };

Color System
============

Dynamic Color Cycling
---------------------

All PixelFlow variants feature sophisticated color cycling:

.. code-block:: cpp

   void updateColorScheme(int colorIndex) {
       switch (colorIndex % 6) {
           case 0:  // Blue-green spectrum
               col1.r(0);
               col1.g(255 - rand() % 100);
               col1.b(255 - rand() % 200);
               break;
               
           case 1:  // Red-blue spectrum  
               col1.g(0);
               col1.b(255 - rand() % 100);
               col1.r(255 - rand() % 200);
               break;
               
           case 2:  // Green-red spectrum
               col1.b(0);
               col1.r(255 - rand() % 100);
               col1.g(255 - rand() % 200);
               break;
               
           case 3:  // Pure blue variations
               col1.r(0); col1.g(0);
               col1.b(255 - rand() % 200);
               break;
               
           case 4:  // Pure red variations
               col1.g(0); col1.b(0);
               col1.r(255 - rand() % 200);
               break;
               
           case 5:  // Pure green variations
               col1.r(0); col1.b(0);
               col1.g(255 - rand() % 200);
               break;
       }
   }

**PixelFlow3 Special Case**:
PixelFlow3 uses a simplified 2-mode system:

.. code-block:: cpp

   switch (counterColChange % 2) {
       case 0:  // Green with intensity variation
           col1.r(0);
           col1.g(255);
           col1.b(150);
           col1 *= (float)(rand() % 100) / 100.0f;
           break;
           
       case 1:  // Blue-red combination
           col1.g(0);
           col1.b(255 - rand() % 100);
           col1.r(255 - rand() % 200);
           break;
   }

IMU Integration (PixelFlow & PixelFlow2)
=======================================

MPU6050 Sensor Usage
--------------------

The original PixelFlow variants demonstrate advanced IMU integration:

.. code-block:: cpp

   class PixelFlow : public CubeApplication {
   private:
       Mpu6050 Imu;                    // MPU6050 IMU sensor
       
   public:
       void spawnParticles() {
           // Get intersection point of gravity vector with cube
           auto imuPoint = Imu.getCubeAccIntersect();
           
           // Determine spawn surface and initial velocity
           Vector3f startSpeed(0, 0, 0);
           switch(getScreenNumber(imuPoint)) {
               case top: case bottom:
                   startSpeed[0] = vx;   // X-Y plane movement
                   startSpeed[1] = vy;
                   break;
               case front: case back:
                   startSpeed[0] = vx;   // X-Z plane movement
                   startSpeed[2] = vy;
                   break;
               case left: case right:
                   startSpeed[1] = vx;   // Y-Z plane movement
                   startSpeed[2] = vy;
                   break;
           }
           
           // Create particle at IMU-determined surface point
           createParticle(imuPoint.cast<float>(), startSpeed);
       }
       
       void updateParticles() {
           for(auto particle : particles) {
               // Apply real-world acceleration from IMU
               Vector3f imuAccel = Imu.getAcceleration();
               Vector3f randomVariation(-0.1f + rand_variation);
               
               particle->acceleration(imuAccel * randomVariation);
               particle->step();
           }
       }
   };

Real-World Physics
-----------------

IMU integration enables real-world physics simulation:

* **Tilt Response**: Particles flow toward the "down" direction based on cube orientation
* **Motion Sensitivity**: Shaking the cube increases particle activity
* **Gravity Simulation**: Particles respond to real gravity direction
* **Surface Selection**: Spawn points determined by which face is "up"

Interactive Controls
====================

PixelFlow3 Controls
------------------

**Button A**: Cycle Color Schemes
   Changes between green and blue-red color modes

**Button Y**: Pause/Unpause
   Freezes particle simulation while preserving state

**No IMU Dependency**: Runs on any platform with matrixserver support

.. code-block:: cpp

   void handleInput() {
       for (auto joystick : joysticks) {
           if (joystick->getButtonPress(0)) {
               counterColChange++;        // Color scheme change
           }
           if (joystick->getButtonPress(3)) {
               isPaused = !isPaused;     // Pause toggle
           }
           joystick->clearAllButtonPresses();
       }
   }

Performance Characteristics
===========================

Particle Density Comparison
---------------------------

* **PixelFlow**: 60 particles/frame - High density for rich fluid effects
* **PixelFlow2**: 60 particles/frame - Same density with optimized management  
* **PixelFlow3**: 30 particles/frame - Balanced for cross-platform performance

Frame Rate and Optimization
---------------------------

All variants run at **40 FPS** with different optimization strategies:

.. code-block:: cpp

   // PixelFlow/PixelFlow2: High particle count with IMU processing
   for (int i = 0; i < 60; i++) {
       // IMU-based particle creation (expensive)
       createIMUParticle();
   }
   
   // PixelFlow3: Lower particle count for broader compatibility
   for (int i = 0; i < 30; i++) {
       // Simple algorithmic particle creation (efficient)  
       createAlgorithmicParticle();
   }

Memory Management
----------------

All variants use efficient smart pointer management:

.. code-block:: cpp

   // Automatic cleanup with STL algorithms
   particles.erase(std::remove_if(particles.begin(), particles.end(),
       [](std::shared_ptr<Drop> p) { return p->getRdyDelete(); }),
       particles.end());

Building and Platform Requirements
==================================

Build Configuration
-------------------

.. code-block:: bash

   # PixelFlow and PixelFlow2 (Raspberry Pi only)
   # Requires MPU6050 IMU sensor and appropriate I2C setup
   make pixelflow    # Original IMU version
   make pixelflow2   # Enhanced IMU version
   
   # PixelFlow3 (All platforms)  
   make pixelflow3   # Cross-platform version

Platform Requirements
--------------------

**PixelFlow & PixelFlow2**:
   * Raspberry Pi with I2C enabled
   * MPU6050 IMU sensor connected via I2C
   * matrixserver running on Raspberry Pi

**PixelFlow3**:
   * Any platform supporting matrixserver
   * Optional USB gamepad/joystick for interaction
   * No sensor hardware requirements

Hardware Setup (PixelFlow/PixelFlow2)
-------------------------------------

MPU6050 I2C connection:
* VCC → 3.3V
* GND → Ground  
* SDA → GPIO 2 (Pin 3)
* SCL → GPIO 3 (Pin 5)

Enable I2C on Raspberry Pi:
.. code-block:: bash

   sudo raspi-config
   # Navigate to Interface Options → I2C → Enable

Advanced Usage
==============

Customization Options
--------------------

**Particle Density**:
.. code-block:: cpp

   // Increase particle count for denser effects
   for (int i = 0; i < 120; i++) {  // Double density
       createParticle();
   }

**Fade Persistence**:
.. code-block:: cpp

   fade(0.95);  // Longer trails
   fade(0.70);  // Shorter trails

**Color Cycling Speed**:
.. code-block:: cpp

   if (counter % 25 == 0) {  // Faster color changes
       counterColChange++;
   }

Physics Tuning
--------------

**IMU Sensitivity** (PixelFlow/PixelFlow2):
.. code-block:: cpp

   // More sensitive to IMU input
   particle->acceleration(Imu.getAcceleration() * -0.2f);
   
   // Less sensitive to IMU input  
   particle->acceleration(Imu.getAcceleration() * -0.05f);

**Particle Lifetime**:
.. code-block:: cpp

   // Longer-lived particles
   if (stepCount > 300) { rdyDelete_ = true; }
   
   // Shorter-lived particles
   if (stepCount > 100) { rdyDelete_ = true; }

Development Workflow
====================

Testing Sequence
----------------

1. **Start with PixelFlow3** for initial development (no hardware dependencies)
2. **Verify particle physics** and visual effects work correctly  
3. **Deploy to Raspberry Pi** with PixelFlow/PixelFlow2 for IMU integration
4. **Fine-tune IMU sensitivity** and surface dynamics

The PixelFlow series demonstrates the evolution from simple particle systems to sophisticated sensor-integrated fluid dynamics, providing excellent examples of both cross-platform development and hardware-specific optimization in LED cube applications.