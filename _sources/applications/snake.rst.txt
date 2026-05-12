=====
Snake
=====

Snake is a sophisticated 3D implementation of the classic Snake game, adapted for LED Matrix Cube displays. It supports multiple simultaneous players, joystick control, and features advanced 3D navigation mechanics that allow snakes to move seamlessly across cube faces.

Game Features
============

Core Gameplay
------------

* **3D Snake Movement**: Snakes navigate through 3D space and across cube surfaces
* **Multi-Player Support**: Up to 8 simultaneous players with different colors
* **Collision Detection**: Snake-to-snake collisions and self-collision detection
* **Food System**: Randomly placed food items that cause snakes to grow
* **Score Persistence**: High score tracking with file-based storage
* **Dynamic Difficulty**: Snake speed increases as players eat food

3D Navigation System
-------------------

The game features sophisticated 3D movement mechanics:

**Surface Transitions**: Snakes can move seamlessly between cube faces
**Edge Wrapping**: Automatic navigation around cube edges and corners
**Velocity Transformation**: Direction vectors are automatically transformed when crossing edges
**Collision Boundaries**: Proper collision detection across 3D surfaces

Technical Architecture
=====================

Class Structure
--------------

The Snake application consists of multiple interconnected classes:

.. code-block:: cpp

   class Snake : public CubeApplication {
   public:
       Snake();                           // Constructor with 40 FPS
       bool loop() override;              // Main game loop
       
   private:
       std::vector<Player*> players;      // Active players
       std::vector<Food*> food;           // Food items
       int currentHighScore;              // Current high score
   };

Player Class
-----------

Each snake is represented by a ``Player`` object with complex behavior:

.. code-block:: cpp

   class Snake::Player {
   public:
       Player(CubeApplication* renderCube, int joysticknumber, 
              Vector3f setPosition, Vector3f setVelocity, 
              Color setColor, unsigned int length);
       
       void step();                       // Update player state
       void handleJoystick();             // Process input
       void render();                     // Draw snake
       void turnLeft();                   // Left turn logic
       void turnRight();                  // Right turn logic
       
   private:
       std::vector<Vector3f> tail;        // Snake body segments
       Joystick joystick;                 // Controller interface
   };

Food Class
---------

Food items are simple objects that provide growth when consumed:

.. code-block:: cpp

   class Snake::Food {
   public:
       Food(CubeApplication* renderCube, Vector3i setPosition, Color setColor);
       
       void render();                     // Draw food item
       void eat();                        // Mark as consumed
       
   private:
       Vector3i position;                 // 3D position
       Color color;                       // Display color
       bool isEaten;                      // Consumption state
   };

Game Mechanics
=============

Player Movement
--------------

Snake movement in 3D space involves complex coordinate transformations:

**Position Updates**:

.. code-block:: cpp

   void Player::step() {
       accelerate();                      // Apply acceleration
       move();                           // Update position
       warp();                           // Handle edge transitions
       
       // Position clamping and rounding
       for (unsigned int i = 0; i < 3; i++) {
           if (position[i] < 0.01 && position[i] > 0)
               position[i] = 0;
           if (position[i] > VIRTUALCUBEMAXINDEX - 0.01)
               position[i] = VIRTUALCUBEMAXINDEX;
       }
   }

**Edge Navigation**:

The ``warp()`` function handles transitions between cube faces:

.. code-block:: cpp

   void Player::warp() {
       Vector3i currentPosition = iPosition();
       EdgeNumber currentEdge = ca->getEdgeNumber(currentPosition);
       
       if (currentEdge != anyEdge) {
           // Transform velocity vectors based on edge type
           switch (currentEdge) {
               case topLeft:
               case topRight:
               case bottomRight:
               case bottomLeft:
                   std::swap(velocity[2], velocity[0]);
                   break;
               // ... additional edge cases
           }
           
           // Reflect velocity if hitting boundaries
           if ((currentPosition[0] == 0 && velocity[0] < 0) ||
               (currentPosition[0] == VIRTUALCUBEMAXINDEX && velocity[0] > 0))
               velocity[0] *= -1;
       }
   }

Turning Mechanics
---------------

The turning system adapts to the snake's current position on the cube:

.. code-block:: cpp

   void Player::turnLeft() {
       if (!isDying && !ca->isOnEdge(iPosition())) {
           if (position[2] == 0) {                    // Bottom face
               std::swap(velocity[0], velocity[1]);
               velocity[0] = -velocity[0];
           } else if (position[2] == VIRTUALCUBEMAXINDEX) { // Top face
               std::swap(velocity[0], velocity[1]);
               velocity[1] = -velocity[1];
           }
           // ... additional face-specific turning logic
       }
   }

Input System
===========

Joystick Integration
-------------------

Each player can use a dedicated joystick controller:

.. code-block:: cpp

   void Player::handleJoystick() {
       if (joystick.isFound()) {
           float newAxis0 = joystick.getAxis(0);
           if (newAxis0 < 0 && lastAxis0 == 0) {
               turnLeft();
           } else if (newAxis0 > 0 && lastAxis0 == 0) {
               turnRight();
           }
           lastAxis0 = newAxis0;
       } else {
           doKiMove();                    // AI fallback
       }
   }

AI Fallback
----------

Players without joysticks use simple AI:

.. code-block:: cpp

   void Player::doKiMove() {
       int random = rand() % 512;
       if (random == 55) {
           turnLeft();
       } else if (random == 66) {
           turnRight();
       }
   }

Collision System
===============

Multi-layered Collision Detection
--------------------------------

The game implements several collision types:

**Snake-to-Snake Collisions**:

.. code-block:: cpp

   for (auto player : players) {
       for (auto player2 : players) {
           if (player->collidesWith(player2->iPosition()) && 
               player != player2 && !player->getIsDying() &&
               !player2->getIsDying()) {
               player2->die();
               player->grow(player2->getSnakeLength() / 4);
               player->speedUp(1.10);
           }
       }
   }

**Self-Collision Detection**:

.. code-block:: cpp

   Vector3i head = iPosition();
   int colCounter = 0;
   for (auto t : tail)
       if (head == t.cast<int>())
           colCounter++;
   if (colCounter > 1)
       die();

**Food Consumption**:

.. code-block:: cpp

   for (auto f : food) {
       for (auto p : players) {
           if (p->iPosition() == f->getPosition()) {
               p->grow(2);
               p->speedUp(1.05);
               f->eat();
               food.push_back(new Food(this, getRandomPointOnScreen(anyScreen), 
                                      Color::randomBlue() * 2));
           }
       }
   }

Score Management
===============

High Score System
----------------

The game maintains persistent high scores using file I/O:

.. code-block:: cpp

   bool Snake::updateHighScoreFromToFile(int score, std::string filename) {
       std::ifstream highScoreFile(filename);
       
       if (highScoreFile) {
           highScoreFile >> currentHighScore;
       } else {
           // Create new high score file
           std::ofstream newFile(filename);
           newFile << 0;
           currentHighScore = 0;
       }
       
       if (score > currentHighScore) {
           currentHighScore = score;
           std::ofstream updateFile(filename);
           updateFile << currentHighScore;
           return true;  // New high score achieved
       }
       
       return false;
   }

High Score Display
-----------------

New high scores trigger a celebration animation:

.. code-block:: cpp

   if (highScoreTime) {
       Color fontColor = highScoreColor;
       
       if (highScoreTimer/5%2 == 0)
           fontColor = Color::black();     // Blinking effect
       else
           fontColor = highScoreColor;
       
       // Display on all faces
       drawText(top, Vector2i(centered, centered), fontColor, 
                "HIGHSCORE " + std::to_string(currentHighScore));
   }

Visual Effects
=============

Death Animation
--------------

When snakes die, they flash between colors:

.. code-block:: cpp

   if (isDying) {
       if (dieCounter / 40 % 2) {
           color = Color::black();
       } else {
           color = Color::white();
       }
       dieCounter++;
       if (dieCounter >= 200) {
           isDead = true;
       }
   }

Snake Rendering
--------------

Each snake segment is rendered as a 3D pixel:

.. code-block:: cpp

   void Player::render() {
       for (auto t : tail) {
           ca->setPixel3D(t[0], t[1], t[2], color);
       }
   }

Performance Optimization
=======================

Oversampling
-----------

The game uses oversampling to ensure smooth collision detection:

.. code-block:: cpp

   for (int oversampling = 8; oversampling > 0; oversampling--) {
       for (auto player : players) {
           player->step();
           // ... collision detection and rendering
       }
   }

Memory Management
----------------

Efficient food management using STL algorithms:

.. code-block:: cpp

   food.erase(std::remove_if(food.begin(), food.end(), 
                            [](Food *f) { return (f->getIsEaten()); }), 
              food.end());

Build and Execution
==================

The Snake application requires uncommenting in the main CMakeLists.txt:

.. code-block:: cmake

   add_subdirectory(Snake)

Compilation and execution:

.. code-block:: bash

   make snake
   ./snake

Game Controls
============

Default controller mappings:

* **Axis 0 Left/Right**: Turn left/right
* **Multiple Controllers**: Support for up to 8 simultaneous players
* **AI Fallback**: Non-controller players use random AI

This implementation demonstrates advanced 3D game development techniques and provides an engaging multi-player experience on LED Matrix Cube displays.