cmake_minimum_required(VERSION 3.17)
project(FractalViewer)

set(CMAKE_CXX_STANDARD 14)

include_directories(FractalViewer)

add_executable(FractalViewer
        FractalViewer/ArialFont.h
        FractalViewer/Source.cpp FractalViewer/Fractal.cpp FractalViewer/Fractal.h)

# Find SFML and OpenMP
find_package(SFML 2.5 COMPONENTS audio graphics window system REQUIRED)
find_package(OpenMP REQUIRED)

#add_subdirectory(ECS)
target_link_libraries(FractalViewer
        PUBLIC
        sfml-graphics sfml-audio sfml-window sfml-system OpenMP::OpenMP_CXX)