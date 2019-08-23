cmake_minimum_required(VERSION 3.13)
project(Mandelbrot)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenMP)
find_package(CUDA)

if (SDL2_FOUND AND SDL2_IMAGE_FOUND)
    message("SDL2 and SDL2_image found.")
else ()
    message("SDL2 and SDL2_image was not found.")
endif ()

if (CUDA_FOUND)
    message("CUDA found.")
    add_executable(mandelbrot_cu mandelbrot.cu)
    target_link_libraries(mandelbrot_cu ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
else()
    message("CUDA not available")
endif()

add_executable(mandelbrot mandelbrot.c)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIR})
target_link_libraries(mandelbrot ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
