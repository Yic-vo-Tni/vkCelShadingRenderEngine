cmake_minimum_required(VERSION 3.29)

project(VkCelShadingRenderer LANGUAGES CXX )
if (MSVC)
    enable_language(CUDA)
    set(CUDAToolkit_ROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6")
    set(CMAKE_CUDA_COMPILER "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6/bin/nvcc.exe")
    set(CMAKE_CUDA_ARCHITECTURES 86)
    set(CMAKE_CUDA_STANDARD 23)
endif ()

set(CMAKE_CXX_STANDARD 23)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX /utf-8")
#    add_compile_options("/utf-8")
elseif (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -D__USE_MINGW_ANSI_STDIO=1")
endif ()

add_executable(VkCelShadingRenderer main.cpp)
add_executable(sabaMiddleWare sabaMiddleWare.cpp)
add_executable(nSaba saba.cpp)

target_link_libraries(VkCelShadingRenderer PRIVATE renderer)

target_include_directories(sabaMiddleWare PUBLIC
        renderer/third
        subprojects/saba/src
)
target_link_libraries(sabaMiddleWare PRIVATE "Saba")

target_link_libraries(nSaba PRIVATE renderer)

add_subdirectory(subprojects)
add_subdirectory(renderer)
