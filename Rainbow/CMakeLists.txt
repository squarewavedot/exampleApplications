project(Rainbow)

find_package(matrixapplication REQUIRED)

set(MAINSRC
    main.cpp rainbow.cpp rainbow.h)

set(MAINLIBS
        matrixapplication::matrixapplication
)

add_executable(Rainbow ${MAINSRC})
target_link_libraries(Rainbow ${MAINLIBS}) #-l flag

install(TARGETS Rainbow DESTINATION /home/pi/APPS)