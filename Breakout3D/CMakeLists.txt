project(Breakout3D)

find_package(matrixapplication REQUIRED)

set(MAINSRC
        breakoutgame.cpp
        main.cpp
        #aplay.cpp
        )

set(MAINLIBS
        #  libasound.so
        matrixapplication::matrixapplication
        )

add_executable(Breakout3D ${MAINSRC})
target_link_libraries(Breakout3D ${MAINLIBS})

install(TARGETS Breakout3D DESTINATION /home/pi/APPS)