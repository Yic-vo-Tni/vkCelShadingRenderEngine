cmake_minimum_required(VERSION 3.28)
project(Renderer)

file(GLOB_RECURSE SRCS "src/*.h" "src/*.cpp")

add_library(Renderer STATIC ${SRCS})
target_precompile_headers(Renderer PUBLIC ${pch_files})
target_include_directories(Renderer PUBLIC
        ../external
        third
        third/spdlog
        src
)
target_link_libraries(Renderer PUBLIC
        "Vulkan::Vulkan"
        ${glfw_mingw}
)