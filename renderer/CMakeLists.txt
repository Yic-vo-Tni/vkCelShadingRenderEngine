cmake_minimum_required(VERSION 3.28)
project(Renderer)

set(BOOST_ROOT "F:/Framework/boost_1_85_0")
set(BOOST_LIBRARYDIR  "${BOOST_ROOT}/stage/lib")
message("Boost root: ${BOOST_ROOT}")
message("Boost library directory: ${BOOST_LIBRARYDIR}")

file(GLOB_RECURSE SRCS "src/*.h" "src/*.cpp")

add_library(Renderer STATIC ${SRCS})
target_precompile_headers(Renderer PUBLIC ${pch_files})
target_include_directories(Renderer PUBLIC
        ../external
        third
        third/spdlog
        src
        ../subprojects/ImGui
)
target_link_libraries(Renderer PUBLIC
        "Vulkan::Vulkan"
        ${glfw_mingw}
        ws2_32
        "${BOOST_LIBRARYDIR}/libboost_filesystem-mgw13-mt-d-x64-1_85.a"
        "imgui"
        ${tbb12}
)