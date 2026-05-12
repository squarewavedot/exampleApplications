==========
Breakout3D
==========

**Breakout3D** is a full-featured 3D implementation of the classic Breakout arcade game, showcasing advanced game mechanics, physics simulation, and multiplayer functionality within the LED cube environment.

Overview
========

Breakout3D demonstrates:

* **Complete Game Implementation** - Full game loop with pregame, ingame, and postgame states
* **3D Physics System** - Vector-based ball movement with realistic collision detection
* **Multiplayer Support** - Two-player competitive gameplay with joystick controls
* **AI Integration** - Automatic AI opponents when joysticks are unavailable
* **Scoring System** - Point rewards, penalties, and persistent high score tracking
* **Power-ups** - Special abilities including slow motion and rocket ball
* **Advanced UI** - Multi-screen text rendering and game state display

Game Mechanics
==============

Core Gameplay
-------------

The game follows classic Breakout mechanics adapted for 3D space:

1. **Players** control paddles that wrap around the bottom edges of the cube
2. **Balls** bounce in 3D space, reflecting off cube surfaces and player paddles  
3. **Blocks** are arranged on the top face and must be destroyed for points
4. **Lives** are managed through ball respawning and penalty systems
5. **Victory** occurs when all blocks are destroyed or time expires

Game States
-----------

.. code-block:: cpp

   enum GameState { pregame, ingame, postgame };

**Pregame State:**
   - Display instructions and controls
   - Show current high score
   - Wait for player input to start
   - Initialize game objects

**Ingame State:**
   - Active physics simulation
   - Player paddle control
   - Ball-block collision detection
   - Real-time scoring and timer

**Postgame State:**
   - Display final scores and winner
   - Update and save high scores
   - Show victory animations
   - Automatic return to pregame

Implementation Details
======================

Core Architecture
-----------------

.. code-block:: cpp

   class BreakoutGame : public CubeApplication {
   private:
       enum GameState { pregame, ingame, postgame };
       GameState gameState_;
       
       std::vector<Player*> players_;
       std::vector<Ball*> balls_;
       std::vector<Block*> blocks_;
       std::vector<Joystick*> joysticks_;
       
       int remainingSeconds_;
       int currentHighScore;
       
   public:
       BreakoutGame() : CubeApplication(40, "192.168.188.106") {
           reset();
           updateHighScoreFromToFile();
       }
       
       bool loop() override;
   };

Game Object Classes
-------------------

**Player Class:**

.. code-block:: cpp

   class Player {
   private:
       float score_;
       int id_;
       Color color_;
       std::vector<Vector3i> paddlePixels_;
       Joystick* joystick_;
       Ball* lastBall_;
       float vel_;
       
   public:
       Player(CubeApplication* renderCube, int id, Joystick* joystick);
       void step();              // Update player state
       void render();            // Draw paddle
       void doKIMove();          // AI movement logic
       bool collidesWith(Vector3i pos);
   };

**Ball Class:**

.. code-block:: cpp

   class Ball {
   private:
       Vector3f position_;
       Vector3f velocity_;
       Vector3f acceleration_;
       Color color_;
       Player* lastPlayer_;
       float speed_;
       bool isDead_;
       
   public:
       Ball(CubeApplication* renderCube, Vector3f startPosition, 
            Vector3f startVelocity, float speed);
       void step();              // Physics update
       void render();            // Draw ball
       void reflect(Vector3f reflectionVector);
       void die();               // Handle ball death
       void reset();             // Respawn ball
   };

**Block Class:**

.. code-block:: cpp

   class Block {
   private:
       std::vector<Vector3i> blockPixels_;
       Color color_;
       int score_;
       int size_;
       bool isDead_;
       
   public:
       Block(CubeApplication* renderCube, ScreenNumber screenNr, 
             Vector2i topLeftCorner, int size, int score, Color color);
       void render();
       bool collidesWith(Vector3f pos);
       Vector3f centerPosition();
   };

Physics System
==============

Ball Movement
-------------

The physics system implements realistic 3D ball movement:

.. code-block:: cpp

   void Ball::step() {
       if (!isDead_) {
           accelerate();              // Apply acceleration
           move();                    // Update position
           
           // Constrain to cube bounds
           for(int i = 0; i < 3; i++)
               position_[i] = constrain(position_[i], 0.0f, 
                                       (float)VIRTUALCUBEMAXINDEX);
           
           // Handle edge transitions and reflections
           Vector3i currentPosition = iPosition();
           EdgeNumber currentEdge = ca_->getEdgeNumber(currentPosition);
           
           if (currentEdge != anyEdge && currentEdge != lastEdge_) {
               handleEdgeTransition(currentEdge);
           }
       }
   }

Collision Detection
-------------------

**Ball-Paddle Collision:**

.. code-block:: cpp

   void BreakoutGame::playerLoop() {
       for(auto player : players_) {
           for(auto ball : balls_) {
               if(player->collidesWith(ball->iPosition())) {
                   Vector3f collisionVector = ball->iPosition().cast<float>() - 
                                            player->centerPosition().cast<float>();
                   collisionVector[2] = -15;  // Always upward facing
                   
                   ball->reflect(collisionVector);
                   ball->resetSpeed();
                   player->blink(Color::white());
                   
                   // Link player and ball for tracking
                   ball->setLastPlayer(player);
                   player->setLastBall(ball);
               }
           }
       }
   }

**Ball-Block Collision:**

.. code-block:: cpp

   void BreakoutGame::blockLoop() {
       for(auto block : blocks_) {
           for(auto ball : balls_) {
               if(block->collidesWith(ball->position())) {
                   // Award points to last player who touched ball
                   if(ball->lastPlayer() != NULL)
                       ball->lastPlayer()->addToScore(block->score());
                   
                   // Calculate reflection vector based on collision angle
                   Vector3f incidentVec3 = ball->position() - block->centerPosition();
                   Vector3f reflectionVector = calculateReflection(incidentVec3);
                   
                   ball->reflect(reflectionVector);
                   block->die();
               }
           }
       }
   }

Multiplayer Features
====================

Player Management
-----------------

Breakout3D supports two players with automatic AI fallback:

.. code-block:: cpp

   void BreakoutGame::reset() {
       players_.clear();
       joysticks_.clear();
       
       // Initialize controllers and players
       joysticks_.push_back(new Joystick(0));  // Player 1
       joysticks_.push_back(new Joystick(1));  // Player 2
       
       players_.push_back(new Player(this, 0, joysticks_[0]));
       players_.push_back(new Player(this, 1, joysticks_[1]));
       
       // Spawn balls for each player
       spawnBallForPlayer(0);
       spawnBallForPlayer(1);
   }

AI System
---------

When joysticks are not available, AI automatically controls players:

.. code-block:: cpp

   void Player::doKIMove() {
       if(lastBall_ != NULL) {
           Vector3i ballPos = lastBall_->iPosition();
           int xPosBall = calculateBallScreenPosition(ballPos);
           
           // Add randomization for realistic AI behavior
           xPosBall += 10 - rand() % 20;
           
           // Move paddle toward ball position
           if(xPosBall < pos_)
               vel_ = -1;
           else if(xPosBall > pos_)
               vel_ = 1;
           else
               vel_ = 0;
       }
   }

Power-ups and Special Features
==============================

Slow Motion
-----------

Players can activate slow motion by holding the R shoulder button:

.. code-block:: cpp

   if(joystick_->getButton(7) && score_ > 0) {
       lastBall_->setSpeed(0.5);    // Half speed
       addToScore(-0.025);          // Continuous score cost
   } else {
       lastBall_->resetSpeed();     // Normal speed
   }

Rocket Ball
-----------

Players can fire rocket balls using the B button:

.. code-block:: cpp

   if(joystick_->getButtonPress(1) && score_ > 100) {
       lastBall_->velocity(Vector3f(0, 0, -1));  // Straight down
       lastBall_->resetSpeed();
       addToScore(-100);                         // High score cost
   }

Scoring System
==============

Point Values
------------

* **Block Destruction**: +25 points per block
* **Ball Miss**: -10 points (penalty)
* **Slow Motion Use**: -0.025 points per frame
* **Rocket Ball**: -100 points per use

High Score Persistence
----------------------

.. code-block:: cpp

   bool BreakoutGame::updateHighScoreFromToFile(int score, string filename) {
       std::fstream highScoreFile;
       highScoreFile.open(filename.data(), std::fstream::binary | std::fstream::in);
       
       if (highScoreFile) {
           highScoreFile >> currentHighScore;
       } else {
           // Create file with default score
           highScoreFile.open(filename.data(), 
                             std::fstream::binary | std::fstream::out | std::fstream::trunc);
           highScoreFile << 0;
       }
       
       if (score > currentHighScore) {
           currentHighScore = score;
           // Write new high score
           highScoreFile.close();
           highScoreFile.open(filename.data(), 
                             std::fstream::binary | std::fstream::out | std::fstream::trunc);
           highScoreFile << currentHighScore;
           return true;  // New high score achieved
       }
       
       return false;
   }

User Interface
==============

Multi-Screen Display
--------------------

The game utilizes all six cube faces for information display:

.. code-block:: cpp

   // Display player scores on multiple screens
   int scrNrCounter = 0;
   for(auto player : players_) {
       drawText((ScreenNumber)scrNrCounter, Vector2i(0, 58), 
               player->color(), std::to_string(player->getId()) + ": ");
       drawText((ScreenNumber)scrNrCounter, Vector2i(8, 58), 
               Color::white(), std::to_string(player->score()));
       
       // Mirror on opposite screen
       drawText((ScreenNumber)(scrNrCounter + 2), Vector2i(0, 58), 
               player->color(), std::to_string(player->getId()) + ": ");
       scrNrCounter++;
   }

Game Instructions
-----------------

During pregame, instructions are displayed on multiple faces:

.. code-block:: cpp

   std::string playtext = "PRESS A TO PLAY";
   std::string rbutton = "R:  SLOMO";
   std::string bbutton = "B: ROCKET";
   
   for(int i = 0; i < 4; i++) {
       drawText((ScreenNumber)i, Vector2i(CharacterBitmaps::centered, 20), 
               Color::white(), playtext);
       drawText((ScreenNumber)i, Vector2i(CharacterBitmaps::centered, 30), 
               Color::white() * 0.5, rbutton);
       drawText((ScreenNumber)i, Vector2i(CharacterBitmaps::centered, 36), 
               Color::white() * 0.5, bbutton);
   }

Building and Running
====================

Build Configuration
-------------------

.. code-block:: bash

   # Build Breakout3D
   mkdir -p build && cd build
   cmake .. && make breakout3d
   
   # Run with matrixserver
   ./breakout3d

Controls
--------

**Player 1 & 2:**
   * **Left Stick**: Move paddle left/right
   * **A Button**: Start game (pregame state)
   * **B Button**: Rocket ball (costs 100 points)
   * **R Shoulder**: Slow motion (costs points continuously)
   * **Select**: Reset game

Performance
-----------

* **Frame Rate**: 40 FPS for smooth physics and responsive controls
* **Server Connection**: Connects to ``192.168.188.106`` by default
* **Memory Usage**: Efficient object pooling and cleanup

Advanced Features
=================

Edge Wrapping
-------------

Paddles wrap around cube edges seamlessly, with collision detection spanning multiple faces.

Physics Accuracy  
----------------

The physics system includes:
* Realistic ball acceleration and velocity
* Proper collision reflection calculations
* Edge case handling for corner bounces
* Smooth interpolation for sub-pixel positioning

State Persistence
-----------------

Game state is preserved between sessions:
* High scores stored in persistent files
* Game settings maintained across runs
* Player statistics tracked

This comprehensive implementation demonstrates advanced game development techniques within the LED cube framework, providing an engaging multiplayer experience with sophisticated physics and user interface design.