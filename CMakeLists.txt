cmake_minimum_required(VERSION 3.13)
project(Mandelbrot C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenMP)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIR})

include(CheckLanguage)
check_language(CUDA)
if (CMAKE_CUDA_COMPILER)
    message("CUDA compiler found.")
    enable_language(CUDA)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -gencode arch=compute_61,code=sm_61")
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -g --compiler-options -Wall")
    add_executable(mandelbrot_cu mandelbrot.cu)
    target_link_libraries(mandelbrot_cu m ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
else()
    message("CUDA not available")
endif()

add_executable(mandelbrot mandelbrot.c)
target_link_libraries(mandelbrot m ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})

if (OpenMP_C_FOUND)
    message("OpenMP found")
    target_link_libraries(mandelbrot ${OpenMP_C_LIBRARIES})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -DUSE_OPENMP")
endif()
