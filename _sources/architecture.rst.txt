============
Architecture
============

This section provides a comprehensive overview of the LED Matrix Cube framework architecture, application patterns, and design principles used throughout the example applications.

Framework Architecture
======================

The LED Matrix Cube examples are built on a layered architecture that separates concerns and provides flexibility across different hardware platforms:

.. code-block:: text

   ┌─────────────────────────────────────────┐
   │           Example Applications          │
   │  (CubeTestApp, Snake, PixelFlow, etc.)  │
   ├─────────────────────────────────────────┤
   │         Application Base Classes        │
   │    (CubeApplication, MatrixApplication) │
   ├─────────────────────────────────────────┤
   │         MatrixServer Framework          │
   │   (Connection, Rendering, Input, etc.)  │
   ├─────────────────────────────────────────┤
   │            Server Implementations       │
   │ (Simulator, FPGA, RPI-GPIO, RPI-SPI)   │
   ├─────────────────────────────────────────┤
   │             Hardware Layer              │
   │    (LED Matrices, Controllers, etc.)    │
   └─────────────────────────────────────────┘

Application Base Classes
=======================

The framework provides two primary base classes that encapsulate different display paradigms:

CubeApplication Class
--------------------

``CubeApplication`` is designed for 3D volumetric rendering in cubic LED matrix displays:

**Key Features:**

* **3D Coordinate System**: Direct (x, y, z) pixel addressing
* **Volumetric Rendering**: True 3D space manipulation
* **Surface Rendering**: Text and graphics on cube faces
* **Edge Detection**: Automatic edge and corner handling
* **3D Line Drawing**: Optimized 3D line algorithms

**Common Usage Pattern:**

.. code-block:: cpp

   class MyApp : public CubeApplication {
   public:
       MyApp() : CubeApplication(30) {}  // 30 FPS
       
       bool loop() override {
           clear();                          // Clear 3D space
           
           // 3D rendering operations
           setPixel3D(Vector3i(x, y, z), Color::red());
           drawLine3D(start, end, Color::blue());
           drawText(ScreenNumber::front, pos, Color::white(), "Text");
           
           render();                         // Send frame to server
           return true;                      // Continue running
       }
   };

**Applications Using CubeApplication:**

* :doc:`applications/cubetestapp` - Basic 3D functionality demonstration
* :doc:`applications/snake` - Multi-player 3D Snake game with AI opponents  
* :doc:`applications/blackout3d` - Minimal interactive 3D application template
* :doc:`applications/breakout3d` - Full-featured 3D Breakout game with physics

MatrixApplication Class  
----------------------

``MatrixApplication`` is designed for traditional 2D matrix displays:

**Key Features:**

* **2D Coordinate System**: Standard (x, y) pixel addressing
* **Multi-Screen Support**: Handle multiple connected panels
* **Pixel-Level Control**: Direct manipulation of LED states
* **Screen Management**: Coordinate multiple display surfaces

**Common Usage Pattern:**

.. code-block:: cpp

   class MyApp : public MatrixApplication {
   public:
       MyApp() : MatrixApplication(40) {}  // 40 FPS
       
       bool loop() override {
           // 2D rendering operations
           for(auto screen : screens) {
               screen->setPixel(x, y, r, g, b);
           }
           
           render();                         // Send frame to server
           return true;                      // Continue running
       }
   };

**Applications Using MatrixApplication:**

* :doc:`applications/genetic` - Genetic algorithm color evolution visualization
* :doc:`applications/picture` - Image display with animation support and file watching

Specialized Application Types
============================

Animation and Effects Applications
---------------------------------

Several applications focus on visual effects and animations:

**PixelFlow Series** - :doc:`applications/pixelflow`
   Advanced particle simulation applications with three variants:
   
   * **PixelFlow & PixelFlow2**: IMU-integrated fluid simulation with MPU6050 sensor (Raspberry Pi)
   * **PixelFlow3**: Cross-platform particle system with joystick controls (all platforms)  
   * **Surface Dynamics**: Sophisticated particle flow along cube surfaces with edge transitions
   * **Advanced Physics**: Multi-layer particle system with Particle → SurfaceParticle → Drop hierarchy
   * **Sensor Integration**: Real-world gravity and motion response via IMU
   * **Interactive Controls**: Real-time parameter adjustment and color cycling

**Rainbow**
   Particle system with sensor integration:
   
   * **IMU Integration**: Motion-responsive effects  
   * **Multi-Mode Operation**: Various display modes
   * **Joystick Control**: Interactive parameter adjustment
   * **Color Theory**: HSV color space manipulation


Interactive Game Applications
---------------------------

**Snake** - :doc:`applications/snake`
   Multi-player 3D Snake game with AI opponents:
   
   * **3D Navigation**: Movement across cube surfaces with edge wrapping
   * **Multi-Player Support**: Up to 8 simultaneous players with different colors
   * **AI Integration**: Automatic AI opponents when joysticks unavailable
   * **Food Distribution**: Dynamic food spawning across all cube faces
   * **Score Persistence**: High score tracking and file management

**Breakout3D** - :doc:`applications/breakout3d` 
   Full-featured 3D Breakout game with advanced mechanics:
   
   * **Advanced Physics**: Vector-based ball movement with collision reflection
   * **Multiplayer Support**: Two-player competitive gameplay with joystick controls
   * **Power-ups**: Slow motion (R button) and rocket ball (B button) abilities
   * **AI Integration**: Automatic AI opponents with realistic behavior
   * **Scoring System**: Point rewards, penalties, and persistent high score tracking
   * **Game States**: Complete pregame, ingame, and postgame state management

Framework Core Components
=========================

Connection Management
--------------------

The framework handles connections to matrixserver instances through multiple transport mechanisms:

**Connection Priority:**

1. **TCP Network Connection** (localhost:2017)
2. **IPC Message Queues** (Boost-based)
3. **Unix Domain Sockets** (fallback)

**Automatic Discovery:**

Applications automatically attempt to connect to available server instances and gracefully fall back through connection methods.

Frame Rate Control
-----------------

All applications specify target frame rates in their constructors:

.. code-block:: cpp

   // Examples of different frame rates
   CubeApplication(20)   // 20 FPS - Blackout3D
   CubeApplication(30)   // 30 FPS - CubeTestApp  
   CubeApplication(40)   // 40 FPS - Snake, PixelFlow

The framework automatically:

* **Regulates timing** to maintain consistent frame rates
* **Handles load balancing** when CPU cannot maintain target rate
* **Provides performance monitoring** through load tracking

Input Processing
---------------

**Joystick Integration:**

.. code-block:: cpp

   class Joystick {
   public:
       bool getButtonPress(int button);     // Button state changes
       int getAxis(int axis);               // Axis values (-1, 0, 1)
       void clearAllButtonPresses();       // Reset button states
   };

**Multi-Controller Support:**

Applications can support multiple simultaneous controllers:

.. code-block:: cpp

   std::vector<Joystick*> joysticks;
   joysticks.push_back(new Joystick(0));  // Player 1
   joysticks.push_back(new Joystick(1));  // Player 2

**IMU Integration (Raspberry Pi):**

.. code-block:: cpp

   auto acceleration = Imu.getAcceleration();  // 3D acceleration vector
   // Use IMU data to influence rendering

Hardware Abstraction
====================

Server Implementation Abstraction
---------------------------------

Applications work transparently with different server implementations:

**Development Servers:**

* ``server_simulator`` - Software rendering for development
* Cross-platform compatibility for testing

**Hardware Servers:**

* ``server_FPGA`` - FTDI USB interface for FPGA systems
* ``server_FPGA_RPISPI`` - Raspberry Pi SPI interface  
* ``server_RGBMatrix`` - Direct Raspberry Pi GPIO control

**Connection Transparency:**

Applications use the same API regardless of server implementation, allowing development on simulator and deployment on hardware without code changes.

Display Abstraction
------------------

The framework abstracts different display configurations:

**3D Cube Configurations:**

* True volumetric displays (LED cubes)
* Six-panel cube arrangements
* Custom 3D geometries

**2D Matrix Configurations:**

* Single panel displays
* Tiled multi-panel arrays
* Various resolutions (8x8 to 64x64+)

Design Patterns
==============

Application Lifecycle Pattern
----------------------------

All applications follow a consistent lifecycle:

.. code-block:: cpp

   class Application {
   public:
       Application(int fps);           // Constructor with frame rate
       virtual bool loop() = 0;       // Main render loop
       
   protected:
       void clear();                  // Clear display buffer
       void render();                 // Send frame to server
   };

**Loop Implementation Pattern:**

1. **Clear** the previous frame
2. **Update** application state
3. **Render** new content  
4. **Send** frame to display
5. **Return** continuation flag

Resource Management Pattern
--------------------------

**RAII Principles:**

* Automatic resource cleanup in destructors
* Smart pointers for dynamic allocations
* Exception-safe resource handling

**Memory Management:**

.. code-block:: cpp

   // Preferred: Smart pointers
   std::shared_ptr<Drop> particle = std::make_shared<Drop>(...);
   std::vector<std::shared_ptr<Drop>> particles;
   
   // Automatic cleanup with STL algorithms
   particles.erase(std::remove_if(particles.begin(), particles.end(),
       [](std::shared_ptr<Drop> p) { return p->shouldDelete(); }),
       particles.end());

State Management Pattern
-----------------------

**Static State Variables:**

Many applications use static variables in the main loop for persistent state:

.. code-block:: cpp

   bool loop() override {
       static int counter = 0;
       static Color currentColor = Color::red();
       static bool isPaused = false;
       
       // Use state variables for animation and control
       counter++;
       // ... rest of loop logic
   }

This pattern provides:

* **Persistent state** across loop iterations
* **Initialization** on first call
* **Memory efficiency** without class member overhead

Performance Optimization
=======================

Frame Rate Optimization
----------------------

**Oversampling Control:**

.. code-block:: cpp

   #define OVERSAMPLING 1
   
   for (int sample = 0; sample < OVERSAMPLING; sample++) {
       // Physics calculations
   }

**Selective Updates:**

.. code-block:: cpp

   if (counter % 2 == 0) {
       // Update only every other frame
       updatePhysics();
   }

**Load Balancing:**

The framework automatically adjusts timing when applications cannot maintain target frame rates.

Memory Optimization
------------------

**Object Pooling:**

Applications reuse objects where possible to minimize allocation overhead.

**Efficient Culling:**

.. code-block:: cpp

   // Remove particles outside bounds
   particles.erase(std::remove_if(particles.begin(), particles.end(),
       [](const Particle& p) { return p.isOutOfBounds(); }),
       particles.end());

**Batch Operations:**

Minimize individual pixel operations by batching updates where possible.

## Advanced Implementation Examples

Real-World Application Patterns
===============================

The following examples demonstrate advanced patterns from actual applications in the framework:

Game State Management (Breakout3D)
----------------------------------

Complex applications benefit from state machine patterns:

.. code-block:: cpp

   class BreakoutGame : public CubeApplication {
   private:
       enum GameState { pregame, ingame, postgame };
       GameState gameState_;
       std::vector<Player*> players_;
       std::vector<Ball*> balls_;
       std::vector<Block*> blocks_;
       
   public:
       bool loop() override {
           switch(gameState_) {
               case pregame:
                   displayInstructions();
                   if(playerPressedStart()) {
                       gameState_ = ingame;
                       initializeGame();
                   }
                   break;
                   
               case ingame:
                   updatePhysics();
                   handleCollisions();
                   checkGameEnd();
                   break;
                   
               case postgame:
                   displayScores();
                   updateHighScores();
                   if(timeout()) gameState_ = pregame;
                   break;
           }
           render();
           return true;
       }
   };

Particle System Architecture (PixelFlow)
-----------------------------------------

Efficient particle systems use smart pointers and STL algorithms:

.. code-block:: cpp

   class PixelFlow : public CubeApplication {
   private:
       class Particle {
           Vector3f position_, velocity_, acceleration_;
           Color color_;
       public:
           virtual void step() = 0;
           Vector3i iPosition() { return Vector3i(round(position_[0]), 
                                                 round(position_[1]), 
                                                 round(position_[2])); }
       };
       
       class Drop : public Particle {
           bool rdyDelete_;
       public:
           void step() override {
               // Physics update with boundary handling
               accelerate(); move();
               if(outOfBounds()) rdyDelete_ = true;
           }
           bool getRdyDelete() { return rdyDelete_; }
       };
       
   public:
       bool loop() override {
           static std::vector<std::shared_ptr<Drop>> particles;
           
           // Spawn new particles
           for(int i = 0; i < 4; i++) {
               particles.push_back(std::make_shared<Drop>(...));
           }
           
           // Update existing particles
           for(auto p : particles) {
               p->step();
               setPixel3D(p->iPosition(), p->color());
           }
           
           // Remove completed particles
           particles.erase(std::remove_if(particles.begin(), particles.end(),
               [](std::shared_ptr<Drop> p) { return p->getRdyDelete(); }),
               particles.end());
               
           render();
           return true;
       }
   };

Multi-Player Game Management (Snake/Breakout3D)
-----------------------------------------------

Supporting multiple players with AI fallback:

.. code-block:: cpp

   class MultiPlayerGame : public CubeApplication {
   private:
       struct Player {
           int id_;
           Joystick* joystick_;
           Color color_;
           float score_;
           bool isAI_;
           
           void updateAI(GameState& state) {
               if(isAI_) {
                   // AI decision making based on game state
                   makeAIMove(state);
               }
           }
       };
       
       std::vector<Player> players_;
       
   public:
       MultiPlayerGame() : CubeApplication(40, "localhost") {
           // Initialize up to 8 players
           for(int i = 0; i < 8; i++) {
               players_.push_back({
                   i,                              // Player ID
                   new Joystick(i),               // Try to get joystick
                   getPlayerColor(i),             // Unique color
                   0.0f,                          // Initial score
                   !joystick->isFound()           // AI if no joystick
               });
           }
       }
       
       void handlePlayerInput() {
           for(auto& player : players_) {
               if(player.isAI_) {
                   player.updateAI(gameState);
               } else {
                   // Process joystick input
                   if(player.joystick_->getButtonPress(0)) {
                       handlePlayerAction(player);
                   }
               }
           }
       }
   };

Physics and Collision Systems (Breakout3D)
------------------------------------------

Advanced physics with vector mathematics:

.. code-block:: cpp

   class Ball {
   private:
       Vector3f position_, velocity_, acceleration_;
       Player* lastPlayer_;
       
   public:
       void reflect(Vector3f reflectionVector) {
           // Revert position to avoid overlap
           position_ -= velocity_;
           
           // Calculate reflection using: d - 2(d·n)n
           velocity_ = velocity_ - 2 * (velocity_.dot(reflectionVector.normalized())) 
                                    * reflectionVector.normalized();
           
           // Apply reflection
           position_ += velocity_;
       }
       
       void handleCollisionWithPlayer(Player* player) {
           Vector3f collisionVector = position_ - player->centerPosition();
           collisionVector[2] = -15;  // Always reflect upward
           
           // Constrain based on screen position
           switch(getScreenNumber(iPosition())) {
               case front: case back:
                   collisionVector[1] = 0;  // No Y-component
                   break;
               case right: case left:
                   collisionVector[0] = 0;  // No X-component  
                   break;
           }
           
           reflect(collisionVector);
           setLastPlayer(player);
       }
   };

Advanced Input Handling
-----------------------

Comprehensive joystick management with multiple controllers:

.. code-block:: cpp

   class InputManager {
   private:
       std::vector<Joystick*> joysticks_;
       
   public:
       InputManager(int maxControllers = 4) {
           for(int i = 0; i < maxControllers; i++) {
               joysticks_.push_back(new Joystick(i));
           }
       }
       
       void processInput() {
           for(auto joystick : joysticks_) {
               if(!joystick->isFound()) continue;
               
               // Button mappings
               if(joystick->getButtonPress(0))  handleAction();
               if(joystick->getButtonPress(3))  handlePause();
               if(joystick->getButtonPress(6))  handleSpecialLeft();
               if(joystick->getButtonPress(7))  handleSpecialRight();
               
               // Analog stick input
               float axis = joystick->getAxis(0);
               if(abs(axis) > 0.1f) {  // Deadzone
                   handleMovement(axis * movementSpeed_);
               }
               
               joystick->clearAllButtonPresses();
           }
       }
   };

High Score and Persistence (Breakout3D/Snake)
---------------------------------------------

File-based persistence with error handling:

.. code-block:: cpp

   class ScoreManager {
   private:
       int currentHighScore_;
       std::string filename_;
       
   public:
       bool updateHighScore(int newScore) {
           std::fstream file;
           
           // Try to read existing high score
           file.open(filename_, std::fstream::binary | std::fstream::in);
           if(file.is_open()) {
               file >> currentHighScore_;
               file.close();
           } else {
               // Create new file with default score
               currentHighScore_ = 0;
           }
           
           // Check for new high score
           if(newScore > currentHighScore_) {
               currentHighScore_ = newScore;
               
               // Write new high score
               file.open(filename_, 
                        std::fstream::binary | std::fstream::out | std::fstream::trunc);
               if(file.is_open()) {
                   file << currentHighScore_;
                   file.close();
                   return true;  // New high score!
               }
           }
           
           return false;
       }
   };

This architecture provides a robust foundation for LED matrix applications while maintaining flexibility, performance, and ease of development across different hardware platforms. The real-world examples demonstrate how these patterns scale from simple templates like Blackout3D to complex multi-player games like Breakout3D.