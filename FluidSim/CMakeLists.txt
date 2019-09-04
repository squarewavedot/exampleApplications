project(FluidSim)

find_package(matrixapplication REQUIRED)

set(MAINSRC
    main.cpp fluidsim.cpp fluidsim.h)

set(MAINLIBS
        matrixapplication::matrixapplication
)

add_executable(FluidSim ${MAINSRC})
target_link_libraries(FluidSim ${MAINLIBS})

install(TARGETS FluidSim DESTINATION /home/pi/APPS)