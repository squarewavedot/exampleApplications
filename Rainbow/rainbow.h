#ifndef RAINBOW_H
#define RAINBOW_H

#include "CubeApplication.h"
#include "Joystick.h"
#include <vector>
#include <Mpu6050.h>

class Rainbow : public CubeApplication{
public:
    Rainbow();
    bool loop();
private:
    class Particle;
    class Drop;
    Mpu6050 Imu;
    std::vector<Color> allTheColors;
    std::vector<Color> allTheColorsRainbow;
    std::vector<Color> allTheColorsRandom;
    std::vector<Joystick *> joysticks;
};

class Rainbow::Particle{
public:
    Particle(Vector3f pos, Vector3f vel, Vector3f accel, Color col);
    void step();
    void accelerate();
    void move();

    Vector3f position();
    Vector3f velocity();
    Vector3f acceleration();

    Vector3i iPosition();
    Vector3i iVelocity();
    Vector3i iAcceleration();

    void position(Vector3f pos);
    void velocity(Vector3f vel);
    void acceleration(Vector3f accel);

    Color color();
    void color(Color Col);
protected:
    Vector3f position_;
    Vector3f velocity_;
    Vector3f acceleration_;
    Color color_;
};

class Rainbow::Drop : public Particle {
public:
    Drop(Vector3i maxPos, Vector3f pos, Vector3f vel, Vector3f accel, Color col);
    void step();
    bool getRdyDelete();
private:
    float vxOld_;
    float vyOld_;
    Vector3i maxPos_;
    bool rdyDelete_;
};


#endif