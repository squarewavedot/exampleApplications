project(PixelFlow)

find_package(matrixapplication REQUIRED)

set(MAINSRC
    main.cpp pixelflow.cpp pixelflow.h)

set(MAINLIBS
        matrixapplication::matrixapplication
)

add_executable(PixelFlow ${MAINSRC})
target_link_libraries(PixelFlow ${MAINLIBS})

install(TARGETS PixelFlow DESTINATION /home/pi/APPS)