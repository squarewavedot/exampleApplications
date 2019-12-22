project(PixelFlow2)

find_package(matrixapplication REQUIRED)

set(MAINSRC
    main.cpp pixelflow2.cpp pixelflow2.h)

set(MAINLIBS
        matrixapplication::matrixapplication
)

add_executable(PixelFlow2 ${MAINSRC})
target_link_libraries(PixelFlow2 ${MAINLIBS})

install(TARGETS PixelFlow2 DESTINATION /home/pi/APPS)