cmake_minimum_required(VERSION 3.28)
project(Renderer)

set(BOOST_ROOT "F:/Framework/boost_1_85_0")
set(BOOST_LIBRARYDIR  "${BOOST_ROOT}/stage/lib")
message("Boost root: ${BOOST_ROOT}")
message("Boost library directory: ${BOOST_LIBRARYDIR}")

file(GLOB_RECURSE SRCS "src/*.h" "src/*.cpp")
file(GLOB wx_lib "../external/wxWidget/*.a")

FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/sfml"
)

FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG v4.0.0
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/flecs"
)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.4.2
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/assimp"
)

FetchContent_Declare(
        caches
        GIT_REPOSITORY https://github.com/vpetrigo/caches.git
        GIT_TAG v0.1.0
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/caches"
)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.5
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/benchmark"
)


#FetchContent_Declare(
#        opencv
#        GIT_REPOSITORY https://github.com/opencv/opencv.git
#        GIT_TAG 4.10.0
#        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/opencv"
#)

set(BENCHMARK_ENABLE_TESTING OFF)

FetchContent_GetProperties(sfml)
if (NOT sfml_POPULATED)
    FetchContent_Populate(sfml)
    add_subdirectory(${sfml_SOURCE_DIR} ${sfml_BINARY_DIR})
    link_directories(${sfml_BINARY_DIR}/lib)
endif ()

FetchContent_GetProperties(flecs)
if (NOT flecs_POPULATED)
    FetchContent_Populate(flecs)
    add_subdirectory(${flecs_SOURCE_DIR} ${flecs_BINARY_DIR})
    link_directories(${flecs_BINARY_DIR}/lib)
endif ()

FetchContent_GetProperties(assimp)
if (NOT assimp_POPULATED)
    FetchContent_Populate(assimp)
    add_subdirectory(${assimp_SOURCE_DIR} ${assimp_BINARY_DIR})
    link_directories(${assimp_BINARY_DIR}/lib)
endif ()

FetchContent_GetProperties(caches)
if (NOT caches_POPULATED)
    FetchContent_Populate(caches)
endif ()

FetchContent_GetProperties(benchmark)
if (NOT benchmark_POPULATED)
    FetchContent_Populate(benchmark)
    add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})
    link_directories(${benchmark_BINARY_DIR}/lib)
endif ()

#FetchContent_GetProperties(opencv)
#if (NOT opencv_POPULATED)
#    FetchContent_Populate(opencv)
#    add_subdirectory(${opencv_SOURCE_DIR} ${opencv_BINARY_DIR})
#    link_directories(${opencv_BINARY_DIR}/lib)
#endif ()

add_library(Renderer STATIC ${SRCS})
target_precompile_headers(Renderer PUBLIC ${pch_files})
target_include_directories(Renderer PUBLIC
        ../external
        third
        third/spdlog
        src
        ../subprojects/ImGui
        third/sfml/include
        third/wxWidget
#        third/opencv/include
#        third/opencv/modules/core/include
)
target_link_libraries(Renderer PUBLIC
        "Vulkan::Vulkan"
        ${glfw_mingw}
        ws2_32
        "${BOOST_LIBRARYDIR}/libboost_filesystem-mgw13-mt-d-x64-1_85.a"
        "${BOOST_LIBRARYDIR}/libboost_locale-mgw13-mt-d-x64-1_85.a"
        "imgui"
        ${tbb12}
        sfml-system
        sfml-audio
        flecs::flecs_static
        ${wx_lib}
        assimp::assimp
#        benchmark::benchmark
)

file(GLOB_RECURSE DLL_FILES
        "${sfml_BINARY_DIR}/*.dll"
        "${assimp_BINARY_DIR}/*.dll"
#        "${opencv_BINARY_DIR}/*.dll"
)

foreach (dll_file IN LISTS DLL_FILES)
    add_custom_command(TARGET Renderer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${dll_file} ${CMAKE_BINARY_DIR}
    )
endforeach ()


