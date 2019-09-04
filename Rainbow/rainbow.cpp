#include "rainbow.h"
#include <cmath>

#include <iostream>
#include <algorithm>
#include <cctype>
#include <memory>

#define PI 3.14159265

#define OVERSAMPLING 1

Color ColorFade(Color col1, Color col2, float progress) {
    Color returnColor;
    returnColor.r((uint8_t) (col1.r() + (col2.r() - col1.r()) * progress / 100));
    returnColor.g((uint8_t) (col1.g() + (col2.g() - col1.g()) * progress / 100));
    returnColor.b((uint8_t) (col1.b() + (col2.b() - col1.b()) * progress / 100));
    /*std::cout << "Progress: " << (float)progress << " Faktor: " << (float)progress/100 << std::endl;
    std::cout << "R Col Old: " << (int)col1.r() << " R Col New: " << (int)col2.r() << " R Col Return: " << (int)returnColor.r() << std::endl;
    std::cout << "G Col Old: " << (int)col1.g() << " G Col New: " << (int)col2.g() << " G Col Return: " << (int)returnColor.g() << std::endl;
    std::cout << "B Col Old: " << (int)col1.b() << " B Col New: " << (int)col2.b() << " B Col Return: " << (int)returnColor.b() << std::endl;
*/
    return returnColor;
}

Rainbow::Rainbow() : CubeApplication(40) {
    joysticks.push_back(new Joystick(0));
    joysticks.push_back(new Joystick(1));
    joysticks.push_back(new Joystick(2));
    joysticks.push_back(new Joystick(3));

    allTheColors.push_back(Color(255 - rand() % 100, 0, 0));
    allTheColors.push_back(Color(255, 0, 0));
    allTheColors.push_back(Color(255, 255, 0));
    allTheColors.push_back(Color(0, 255, 0));
    allTheColors.push_back(Color(0, 255, 255));
    allTheColors.push_back(Color(0, 0, 255));
    allTheColors.push_back(Color(255, 0, 255));
    allTheColors.push_back(Color(255, 255, 255));
    allTheColors.push_back(Color(0, 0, 0));
    // std::cout << "allTheColors.size " << allTheColors.size() << std::endl;


    allTheColorsRainbow.push_back(Color(255, 0, 0));
    allTheColorsRainbow.push_back(Color(255, 255, 0));
    allTheColorsRainbow.push_back(Color(0, 255, 0));
    allTheColorsRainbow.push_back(Color(0, 255, 255));
    allTheColorsRainbow.push_back(Color(0, 0, 255));
    allTheColorsRainbow.push_back(Color(255, 0, 255));
    //std::cout << "allTheColorsRainbow.size " << allTheColorsRainbow.size() << std::endl;

    allTheColorsRandom.push_back(Color(rand()%127+127, 0, 0));
    allTheColorsRandom.push_back(Color(0,rand()%127+127, 0));
    allTheColorsRandom.push_back(Color(0,0,rand()%127+127));
    allTheColorsRandom.push_back(Color(rand()%127+127, rand()%127+127, 0));
    allTheColorsRandom.push_back(Color(0,rand()%127+127, rand()%127+127));
    allTheColorsRandom.push_back(Color(rand()%127+127,0,rand()%127+127));
    allTheColorsRandom.push_back(Color(rand()%127+127,rand()%127+127,rand()%127+127));
    allTheColorsRandom.push_back(Color(rand()%255, 0, 0));
    allTheColorsRandom.push_back(Color(0,rand()%255, 0));
    allTheColorsRandom.push_back(Color(0,0,rand()%255));
    allTheColorsRandom.push_back(Color(rand()%255, rand()%255, 0));
    allTheColorsRandom.push_back(Color(0,rand()%255, rand()%255));
    allTheColorsRandom.push_back(Color(rand()%255,0,rand()%255));
    allTheColorsRandom.push_back(Color(rand()%255,rand()%255,rand()%255));
}

bool Rainbow::loop() {
    static std::vector<std::shared_ptr<Drop>> rdrops;
    static int stepCounter = 0;
    static int counterColChange = 0;
    static Color col1(255, 0, 0);
    static Color col1Old = col1;
    static Color col2Old = col1;
    static Color col1BeforeBlack = col1;
    static Color col1RainbowOld = col1;
    static Color col1RainbowNew = col1;
    static bool isPaused = false;
    static bool isBlacked = false;
    static bool isBlackedDone = false;
    static int countBlacked = 0;
    static int countRainbow = 0;
    static uint8_t currentRedValue = 127;
    static uint8_t currentGreenValue = 127;
    static uint8_t currentBlueValue = 127;
    static int valueColorChangePerLoop = 2;
    static int colorMode = 1;
    static int colorModeOld = 0;
    static int rainbowSpeedFactor = 2;
    static int rainbowSpeedFactorMax = 10;
    static float colorChangeSpeedFactor = 1.0f;
    static float colorChangeSpeedFactorMax = 2.0f;
    static int colorFadeNumber = 0;
    static int counterStepRainbow = 0;
    static int counterButton3 = 0;
    static int counterButton3Time = 3000; // milli Seconds
    static int counterPulseTime = 2000; // mS
    static int tempCounterPulseTime = 2000; // mS
    static int lengthPulseTime = 100; //  mS
    static int lengthPulseTimeMax = 5000; //  mS
    static int counterPulseTimeMax = 50000; // milli Seconds
    static int counterPulse = 0; // milli Seconds
    static int counterPulse2 = 0; // milli Seconds
    static int counterBackColorPulse = 0; // milli Seconds

    // Colormode 4 Pulse 2
    static int counterPulse3 = 0; //
    static int counterPulseShort = 100; // in ms
    static int counterPulseMiddle = 500; // in ms
    static int counterPulseLong = 1000; //  in ms
    static int counterPulseLongStart = 0; //  in ms
    static bool trigerAxsis0 = 0; //  in ms
    auto imuPoint = Imu.getAcceleration();
    auto imuPointOld = Imu.getAcceleration();



    // Button X -> Pause
    if (joysticks.at(0)->getButtonPress(3)) {
        isPaused = !isPaused;
        std::cout << "isPaused: " << isPaused << std::endl;
    }
    if (isPaused)
        return true;

    colorModeOld =colorMode;
    // Button Y -> Mode
    if (joysticks.at(0)->getButtonPress(4)) {
        colorMode++;
        std::cout << "colorMode: " << colorMode << std::endl;
    }

    // Rainbow Color
    counterStepRainbow++;
    if (counterStepRainbow % rainbowSpeedFactor == 0) {
        countRainbow++;
    }

    if (countRainbow >= 100) {
        countRainbow = 0;
        counterStepRainbow = 0;
        colorFadeNumber++;
        col1RainbowOld = col1;
    }
    if (colorFadeNumber >= allTheColorsRainbow.size()) {
        colorFadeNumber = 0;
    }
    col1RainbowNew = allTheColorsRainbow.at(colorFadeNumber);


    switch (colorMode) {
        // Color Mode
        // Button 0 (A) for Color out of Array
        // Axis 0 (Up Down) for Red Value in-/decrease
        // Axis 1 (Left Right) for Green Value in-/decrease
        // Button 6 (left Shoulder) for Blue Value decreasef
        // Button 7 (right Shoulder) for Blue Value increase
        case 0:
            if (joysticks.at(0)->getButtonPress(0)) {
                counterColChange++;
                if (counterColChange >= allTheColors.size()) {
                    counterColChange = 0;
                }
                currentRedValue = allTheColors.at(counterColChange).r();
                currentGreenValue = allTheColors.at(counterColChange).g();
                currentBlueValue = allTheColors.at(counterColChange).b();
            }
            if (joysticks.at(0)->getAxis(0) == 1 && currentRedValue < 256 - valueColorChangePerLoop) {
                currentRedValue += valueColorChangePerLoop;
            } else if (joysticks.at(0)->getAxis(0) == -1 && currentRedValue > -1 + valueColorChangePerLoop) {
                currentRedValue -= valueColorChangePerLoop;
            }
            if (joysticks.at(0)->getAxis(1) == -1 && currentGreenValue < 256 - valueColorChangePerLoop) {
                currentGreenValue += valueColorChangePerLoop;
            } else if (joysticks.at(0)->getAxis(1) == 1 && currentGreenValue > -1 + valueColorChangePerLoop) {
                currentGreenValue -= valueColorChangePerLoop;
            }
            if (joysticks.at(0)->getButton(7) == 1 && currentBlueValue < 256 - valueColorChangePerLoop) {
                currentBlueValue += valueColorChangePerLoop;
            }
            if (joysticks.at(0)->getButton(6) == 1 && currentBlueValue > -1 + valueColorChangePerLoop) {
                currentBlueValue -= valueColorChangePerLoop;
            }

            col1.r(currentRedValue);
            col1.g(currentGreenValue);
            col1.b(currentBlueValue);

            break;
        case 1:
            // Rainbow Mode
            // Button 6 (left Shoulder) for velocity decrease
            // Button 7 (right Shoulder) for velocity increase

            if (joysticks.at(0)->getButtonPress(7) == 1 && rainbowSpeedFactor < rainbowSpeedFactorMax) {
                rainbowSpeedFactor++;
                //std::cout << "rainbowSpeedFactor: " << rainbowSpeedFactor << " button: " << joysticks.at(0)->getButton(7) << " rTrigButton7: " << rTrigButton7 << std::endl;
            }
            if (joysticks.at(0)->getButtonPress(6) == 1 && rainbowSpeedFactor > 1) {
                rainbowSpeedFactor--;
                //std::cout << "rainbowSpeedFactor: " << rainbowSpeedFactor << " button: " << joysticks.at(0)->getButton(6) << " rTrigButton6: " << rTrigButton6 << std::endl;
            }
            if (joysticks.at(0)->getAxis(0) == 1 && colorChangeSpeedFactor < colorChangeSpeedFactorMax) {
                colorChangeSpeedFactor += 0.1f;
                //std::cout << "colorChangeSpeedFactor: " << colorChangeSpeedFactor << std::endl;

            } else if (joysticks.at(0)->getAxis(0) == -1 && colorChangeSpeedFactor > 0.0f) {
                colorChangeSpeedFactor -= 0.1f;
                //std::cout << "colorChangeSpeedFactor: " << colorChangeSpeedFactor << std::endl;
            }
            if (colorChangeSpeedFactor < 0.1f) {
                colorChangeSpeedFactor = 0.0f;
            }

            col1 = ColorFade(col1RainbowOld, col1RainbowNew, countRainbow);
            break;
        case 2:
            // Pulse -> Trial stuff
            if (joysticks.at(0)->getButtonPress(7) == 1 && counterPulseTime < counterPulseTimeMax) {
                counterPulseTime += 100;
                std::cout << "counterPulseTime: " << counterPulseTime << std::endl;
            }

            if (joysticks.at(0)->getButtonPress(6) == 1 && counterPulseTime > 0) {
                counterPulseTime -= 100;
                std::cout << "counterPulseTime: " << counterPulseTime << std::endl;
            }

            if (joysticks.at(0)->getAxis(0) == 1 && lengthPulseTime < lengthPulseTimeMax) {
                lengthPulseTime += 100;
                std::cout << "lengthPulseTime: " << lengthPulseTime << std::endl;

            } else if (joysticks.at(0)->getAxis(0) == -1 && lengthPulseTime > 0.0f) {
                lengthPulseTime -= 100;
                std::cout << "lengthPulseTime: " << lengthPulseTime << std::endl;
            }
            //counterPulseTime

            if (colorChangeSpeedFactor <= 0.1f) {
                counterPulse++;
            }
            tempCounterPulseTime = counterPulseTime * ((float) ((rand() % 70)) / 100.0f + 0.7f);
            if ((float)(counterPulse  / DEFAULTFPS) >=  tempCounterPulseTime)  {
                std::cout << "Hallo" << std::endl;
                colorChangeSpeedFactor = 0.4f;
                colorChangeSpeedFactor *= (float) ((rand() % 70)) / 100.0f + 0.7f;
                counterPulse = 0;
            }
            if (colorChangeSpeedFactor > 0.1f) {
                counterPulse2++;
            }
            if ((counterPulse2 * 1000 / DEFAULTFPS) >= lengthPulseTime) {
                colorChangeSpeedFactor = 0.0f;
                counterPulse2 = 0;
            }
            col1 = ColorFade(col1RainbowOld, col1RainbowNew, countRainbow);
            /*std::cout << "counterPulse: " << counterPulse << std::endl;
            std::cout << "counterPulse2: " << counterPulse2 << std::endl;
            std::cout << "colorChangeSpeedFactor: " << (float) colorChangeSpeedFactor << std::endl;
            std::cout << "color: " << col1 << std::endl;*/


            break;
        case 3:
            if (colorModeOld != colorMode){
                std::cout <<  "colorMode: 3 - Pulse" << std::endl;
                counterBackColorPulse = 0;
                colorChangeSpeedFactor = 0.4f;
            }

            if (joysticks.at(0)->getButtonPress(7) == 1 ) {
                counterBackColorPulse++;
            }
            switch (counterBackColorPulse){
                case 0:
                    col1 = Color::black();
                    break;
                case 1:
                    col1 = ColorFade(col1RainbowOld, col1RainbowNew, countRainbow);
                    break;
                case 2:
                    col1 = Color::white();
                    break;
                case 3:
                    col1 = allTheColorsRandom.at(rand() % allTheColorsRandom.size());
                    break;
                default:
                    counterBackColorPulse = 0;
                    break;
            }

            if (joysticks.at(0)->getButton(0)==1) {
                colorChangeSpeedFactor = 0.4f;
                colorChangeSpeedFactor *= (float) ((rand() % 100)) / 100.0f + 1.0f;

                //col1 = ColorFade(col1RainbowOld, col1RainbowNew, countRainbow);
                //col1 = allTheColorsRainbow.at(rand() % 6);
                col1 = allTheColorsRandom.at(rand() % allTheColorsRandom.size());
            }
            break;
        case 4:
            if (colorModeOld != colorMode){
                std::cout <<  "colorMode: 4 - Pulse 2" << std::endl;
                colorChangeSpeedFactor = 1.0f;
                col1 = Color::black();
            }


            if (joysticks.at(0)->getButtonPress(7)==1 && counterPulseLong < 10000) {
                counterPulseLong += 10;
                std::cout <<  "counterPulseLong: " << counterPulseLong << std::endl;
            }
            if (joysticks.at(0)->getButtonPress(6) == 1 && counterPulseLong > 0) {
                counterPulseLong -= 10;
                std::cout <<  "counterPulseLong: " << counterPulseLong << std::endl;
            }


            if (joysticks.at(0)->getAxis(0) == 1 && colorChangeSpeedFactor < 10.0f && trigerAxsis0 == 0) {
                colorChangeSpeedFactor += 0.1f;
                trigerAxsis0 = 1;
                std::cout << "colorChangeSpeedFactor: " << colorChangeSpeedFactor << std::endl;
            }
            if (joysticks.at(0)->getAxis(0) == 0 && trigerAxsis0 == 1){
                trigerAxsis0 = 0;
            }
            if (joysticks.at(0)->getAxis(0) == -1 && colorChangeSpeedFactor > 0.0f && trigerAxsis0 == 0) {
                colorChangeSpeedFactor -= 0.1f;
                trigerAxsis0 = 1;
                std::cout << "colorChangeSpeedFactor: " << colorChangeSpeedFactor << std::endl;
            }
            if (joysticks.at(0)->getAxis(0) == 0 && trigerAxsis0 == 1){
                trigerAxsis0 = 0;
            }
            imuPointOld = imuPoint;
            //std::cout << "imuPointOld: " << imuPointOld << std::endl;
            imuPoint = Imu.getAcceleration();
            std::cout << "imuPoint: " << imuPoint << std::endl;

            counterPulse3++;
            if (joysticks.at(0)->getButtonPress(0)==1 || imuPointOld != imuPoint) {
                counterPulseLongStart = counterPulse3;
                //col1 = ColorFade(col1RainbowOld, col1RainbowNew, countRainbow);
                //col1 = allTheColorsRainbow.at(rand() % 6);
                col1 = allTheColorsRandom.at(rand() % allTheColorsRandom.size());
            }

            if ((counterPulse3-counterPulseLongStart)* 1000  / DEFAULTFPS >= counterPulseLong) {
                //std::cout << (counterPulse3-counterPulseLongStart)* 1000  / DEFAULTFPS << std::endl;
                col1 = Color::black();
            }
            if (col1 == Color::black()){
                counterPulse3 = 0;

            }
            break;
        default:
            colorMode = 0;
            break;
    }

    joysticks.at(0)->clearAllButtonPresses();

/*
    if (isBlacked) {
        if (isBlackedDone == false) {
            countBlacked += rainbowSpeedFactor;
            col1 = ColorFade(col1BeforeBlack, Color::black(), (countBlacked));
        }
        if (countBlacked >= 100) {
            isBlackedDone = true;
            countBlacked = 0;
        }
        if (isBlackedDone == true) {
            col1 = Color::black();
        }
    }*/


    col1 *= (float) ((rand() % 70)) / 100.0f + 0.7f;

    fade(0.85);
    //create new Raindrops
    if(colorChangeSpeedFactor > 0.0f) {
        for (int foo = 0; foo < 30; foo++) {
            float randAngle = rand() % 360;
            float vx = (colorChangeSpeedFactor / OVERSAMPLING) * cos(randAngle * PI / 180);
            float vy = (colorChangeSpeedFactor / OVERSAMPLING) * sin(randAngle * PI / 180);
            rdrops.push_back(
                    std::make_shared<Drop>(Vector3i(VIRTUALCUBEMAXINDEX, VIRTUALCUBEMAXINDEX, VIRTUALCUBEMAXINDEX),
                                           Vector3f(VIRTUALCUBECENTER, VIRTUALCUBECENTER, 0), Vector3f(vx, vy, 0),
                                           Vector3f(0, 0, 0), col1));
        }
    }


//    if (counter%50 == 0) {
//        counterColChange++;
//    }


    for (auto r : rdrops) {
        for (int overSamplingCounter = 0; overSamplingCounter < OVERSAMPLING; overSamplingCounter++) {
            r->step();
            setPixel3D(r->iPosition(), r->color());
        }
    }

    //remove drops from the bottom
    rdrops.erase(
            std::remove_if(rdrops.begin(), rdrops.end(), [](std::shared_ptr<Drop> r) { return (r->getRdyDelete()); }),
            rdrops.end());

    render();
    stepCounter++;

    return true;
}


Rainbow::Drop::Drop(Vector3i maxPos, Vector3f pos, Vector3f vel, Vector3f accel, Color col)
        : Particle(pos, vel, accel, col) {
    maxPos_ = maxPos;
    vxOld_ = 0.0f;
    vyOld_ = 0.0f;
    rdyDelete_ = false;
}

void Rainbow::Drop::step() {
    static float oversamplingFactor = 1.0f / OVERSAMPLING;
    Particle::step(); //Do the physics

    if (position_[0] < 0 || position_[1] < 0 || position_[0] > maxPos_[0] || position_[1] > maxPos_[1]) {
        velocity_[2] = 0.3f * oversamplingFactor;
        acceleration_[2] = (0.02f + ((float) (rand() % 10) / 400.0f)) * oversamplingFactor;
        acceleration_[1] = 0;
        acceleration_[0] = 0;
        if (vxOld_ == 0 && vyOld_ == 0) {
            vxOld_ = velocity_[0];
            vyOld_ = velocity_[1];
        }
        velocity_[0] = 0;
        velocity_[1] = 0;
    }

    if (position_[0] < 0) {
        position_[0] = 0;
        position_[2] = 0;
    }

    if (position_[1] < 0) {
        position_[1] = 0;
        position_[2] = 0;
    }

    if (position_[0] > maxPos_[0]) {
        position_[0] = maxPos_[0];
        position_[2] = 0;
    }

    if (position_[1] > maxPos_[1]) {
        position_[1] = maxPos_[1];
        position_[2] = 0;
    }

    if (position_[2] < 0) {
        position_[2] = 0;
        velocity_[2] *= -1;
    }

    if (position_[2] > maxPos_[2]) {
        position_[2] = maxPos_[2];
        velocity_[0] = vxOld_ * -1;
        velocity_[1] = vyOld_ * -1;
        velocity_[2] = 0;
        acceleration_[1] = 0;
        acceleration_[0] = 0;
        acceleration_[2] = 0;
    }
    if (((velocity_[0] > 0 && position_[0] > VIRTUALCUBECENTER) ||
         (velocity_[0] < 0 && position_[0] < VIRTUALCUBECENTER)) && position_[2] == maxPos_[2]) {
        velocity_[0] = 0;
        vxOld_ = 0;
    }
    if (((velocity_[1] > 0 && position_[1] > VIRTUALCUBECENTER) ||
         (velocity_[1] < 0 && position_[1] < VIRTUALCUBECENTER)) && position_[2] == maxPos_[2]) {
        velocity_[1] = 0;
        vyOld_ = 0;
    }
    if (velocity_[0] == 0 && velocity_[1] == 0 && position_[2] == maxPos_[2]) {
        rdyDelete_ = true;
    }

}

bool Rainbow::Drop::getRdyDelete() {
    return rdyDelete_;
}


Rainbow::Particle::Particle(Vector3f pos, Vector3f vel, Vector3f accel, Color col)
        : position_(pos),
          velocity_(vel),
          acceleration_(accel),
          color_(col) {}

void Rainbow::Particle::step() {
    accelerate();
    move();
}

void Rainbow::Particle::move() {
    position_ += velocity_;
}

void Rainbow::Particle::accelerate() {
    velocity_ += acceleration_;
}

Vector3f Rainbow::Particle::position() {
    return position_;
}

Vector3f Rainbow::Particle::velocity() {
    return velocity_;
}

Vector3f Rainbow::Particle::acceleration() {
    return acceleration_;
}

Vector3i Rainbow::Particle::iPosition() {
    return Vector3i(round(position()[0]), round(position()[1]), round(position()[2]));
}

Vector3i Rainbow::Particle::iVelocity() {
    return Vector3i(round(velocity()[0]), round(velocity()[1]), round(position()[2]));
}

Vector3i Rainbow::Particle::iAcceleration() {
    return Vector3i(round(acceleration()[0]), round(acceleration()[1]), round(acceleration()[2]));
}

void Rainbow::Particle::position(Vector3f pos) {
    position_ = pos;
}

void Rainbow::Particle::velocity(Vector3f vel) {
    velocity_ = vel;
}

void Rainbow::Particle::acceleration(Vector3f accel) {
    acceleration_ = accel;
}

Color Rainbow::Particle::color() {
    return color_;
}

void Rainbow::Particle::color(Color Col) {
    color_ = Col;
}

