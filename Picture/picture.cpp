#include "picture.h"
#include <cmath>

#include <iostream>
#include <algorithm>
#include <cctype>
#include <memory>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
std::string filepath = "/media/usb0/autoload.png";
fs::file_time_type lastModificationTime;
int animationPrescale = 0;


Picture::Picture(int argc, char *argv[]) {
    joysticks.push_back(new Joystick(0));
    joysticks.push_back(new Joystick(1));
    joysticks.push_back(new Joystick(2));
    joysticks.push_back(new Joystick(3));

//    for(int i = 0; i < argc; i++){
//        std::cout << i << ": " << argv[i] << std::endl;
//    }

    if (argc > 1) {
        filepath = std::string(argv[argc - 1]);
        if (argc == 4) {
            if (std::string(argv[1]) == "-s") { //speed
                int temp = std::stoi(argv[2]);
                if (temp >= 0 && temp <= getFps()*4)
                    animationPrescale = temp;
            }
            std::cout << argv[1] << std::endl;
        }
    }

    std::cout << animationPrescale << std::endl;

    loadImage(filepath);
}

bool Picture::loadImage(std::string path) {
    if (autoload.loadImage(path.data())) {
        lastModificationTime = fs::last_write_time(fs::path(path));
        if (autoload.getWidth() == 384 && autoload.getHeight() % 64 == 0) {
            std::cout << "imageload " << path << " successful, size: " << autoload.getWidth() << "x" << autoload.getHeight() << std::endl;
            return true;
        } else {
            std::cout << "image has not the right format, " << autoload.getWidth() << "x" << autoload.getHeight() << std::endl;
        }
    } else {
        std::cout << "image does not exist" << std::endl;
    }
    return false;
}

bool Picture::loop() {
    static int loopcount = 0;
    static int verticalPos = 0;

    if (fs::last_write_time(fs::path(filepath)) > lastModificationTime) {
        std::cout << "file change detected, reloading..." << std::endl;
        loadImage(filepath);
        lastModificationTime = fs::last_write_time(fs::path(filepath));
    }

    clear();

    for (auto joystick : joysticks) {
        if (joystick->getButtonPress(0)) {
            verticalPos += 64;
            if (verticalPos > autoload.getHeight() - 64)
                verticalPos = 0;
            std::cout << "Verticalpos: " << verticalPos << std::endl;
        }
        joystick->clearAllButtonPresses();
    }

    if(animationPrescale > 0){
        if((loopcount % animationPrescale) == 0){
            verticalPos += 64;
            if (verticalPos > autoload.getHeight() - 64)
                verticalPos = 0;
        }
    }

    drawImage(top, Vector2i(0, 0), autoload, Vector2i(0, verticalPos));
    drawImage(left, Vector2i(0, 0), autoload, Vector2i(64, verticalPos));
    drawImage(front, Vector2i(0, 0), autoload, Vector2i(128, verticalPos));
    drawImage(right, Vector2i(0, 0), autoload, Vector2i(192, verticalPos));
    drawImage(back, Vector2i(0, 0), autoload, Vector2i(256, verticalPos));
    drawImage(bottom, Vector2i(0, 0), autoload, Vector2i(320, verticalPos));

    loopcount++;
    render();
    return true;
}
