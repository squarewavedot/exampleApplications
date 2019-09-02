project(genetic)

find_package(matrixapplication REQUIRED)

add_executable(genetic main.cpp genetic.cpp)
target_link_libraries(genetic matrixapplication::matrixapplication)

install(TARGETS genetic DESTINATION /home/pi/APPS)