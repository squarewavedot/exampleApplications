project(Blackout3D)

find_package(matrixapplication REQUIRED)

add_executable(Blackout3D main.cpp Blackout3D.cpp)
target_link_libraries(Blackout3D matrixapplication::matrixapplication)