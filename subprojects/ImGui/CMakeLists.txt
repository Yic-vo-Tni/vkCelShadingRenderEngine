cmake_minimum_required(VERSION 3.29)
project(imgui)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE srcs "*.h" "*.cpp")

add_library(imgui STATIC ${srcs})

target_include_directories(imgui PUBLIC
        ../../external
)

target_link_libraries(imgui "Vulkan::Vulkan")