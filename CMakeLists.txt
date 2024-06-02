cmake_minimum_required(VERSION 3.29)
project(vkCelShadingRenderer)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (WIN32)
        set(CMAKE_WIN32_EXECUTABLE ON)
    endif ()
endif ()

find_package(Vulkan REQUIRED)
find_program(glsl glslc REQUIRED)
find_library(glfw_msvc glfw3 PATHS external/GLFW/msvc)
find_library(glfw_mingw glfw3 PATHS external/GLFW/mingw)

include(FetchContent)

file(GLOB_RECURSE pch_files pch/*.h)

add_executable(vkCelShadingRenderer main.cpp)
target_precompile_headers(vkCelShadingRenderer PUBLIC ${pch_files})

target_include_directories(vkCelShadingRenderer PUBLIC
        external
)
target_link_libraries(vkCelShadingRenderer PUBLIC
        "Renderer"
        "Vulkan::Vulkan"
        ${glfw_mingw}
)

add_subdirectory(renderer)