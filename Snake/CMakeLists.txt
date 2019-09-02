project(Snake)

find_package(matrixapplication REQUIRED)

set(MAINSRC
  snake.cpp
  main.cpp
)

set(MAINLIBS
        matrixapplication::matrixapplication
)

add_executable(Snake ${MAINSRC})
target_link_libraries(Snake ${MAINLIBS})

install(TARGETS Snake DESTINATION /home/pi/APPS)