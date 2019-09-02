project(cubeTestApp)

find_package(matrixapplication REQUIRED)

add_executable(cubetestapp main.cpp CubeTest.cpp)
target_link_libraries(cubetestapp matrixapplication::matrixapplication)