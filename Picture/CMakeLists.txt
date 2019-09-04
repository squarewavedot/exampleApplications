project(Picture)

find_package(matrixapplication REQUIRED)

set(MAINSRC
    main.cpp picture.cpp picture.h)

set(MAINLIBS
        matrixapplication::matrixapplication
        stdc++fs
)

add_executable(Picture ${MAINSRC})
target_link_libraries(Picture ${MAINLIBS} ) #-l flag

install(TARGETS Picture DESTINATION /home/pi/APPS)