#ifndef __SNAKE_H__
#define __SNAKE_H__

#include <CubeApplication.h>
#include <Joystick.h>

#define DEFAULTHIGHSCOREFILE "/home/pi/.snakehighscore"

class Snake : public CubeApplication {
public:
    Snake();

    bool loop();

private:

    bool updateHighScoreFromToFile(int score = 0, std::string filename = DEFAULTHIGHSCOREFILE);

    class Player;

    class Food;

    std::vector<Joystick *> joysticks;
    std::vector<Player *> players;
    std::vector<Food *> food;

    int currentHighScore;
};

class Snake::Player {
public:
    Player(CubeApplication *renderCube, int joysticknumber, Vector3f position, Vector3f velocity, Color color,
           unsigned int length);

    void reset();

    void step();

    void accelerate();

    void move();

    void warp();

    void handleJoystick();

    void doKiMove();

    void render();

    void turnLeft();

    void turnRight();

    bool collidesWith(Vector3i point);

    void grow(unsigned int howMuch);

    void speedUp(float factor);

    void die();

    bool getIsDying();

    bool getIsDead();

    Vector3i iPosition();

    int getSnakeLength();

    Color getDefaultColor();

private:
    std::vector<Vector3f> tail;
    Vector3f position;
    Vector3f velocity;
    Vector3f acceleration;
    Color color;
    Vector3f defaultPosition;
    Vector3f defaultVelocity;
    Color defaultColor;
    bool isDying;
    bool isDead;
    int dieCounter;
    int respawnTimer;
    float speed;
    float defaultSpeed;
    unsigned int snakeLength;
    unsigned int defaultSnakeLength;
    EdgeNumber lastEdge;
    Vector3i lastIPosition;
    Joystick joystick;
    float lastAxis0;
    CubeApplication *ca;
};

class Snake::Food {
public:
    Food(CubeApplication *renderCube, Vector3i position, Color color = Color::red());

    Vector3i getPosition();

    Color getColor();

    bool getIsEaten();

    void eat();

    void render();

protected:
    bool isEaten;
    Vector3i position;
    Color color;
    CubeApplication *ca;
};

#endif
