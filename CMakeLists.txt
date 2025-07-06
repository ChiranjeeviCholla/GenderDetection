cmake_minimum_required(VERSION 3.10)
project(GenderDetection)

find_package(OpenCV REQUIRED)

add_executable(GenderDetection src/main.cpp)
target_link_libraries(GenderDetection ${OpenCV_LIBS})
