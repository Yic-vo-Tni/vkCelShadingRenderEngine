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
)
target_link_libraries(Renderer PUBLIC
        "Vulkan::Vulkan"
        ${glfw_mingw}
        ws2_32
        "${BOOST_LIBRARYDIR}/libboost_filesystem-mgw13-mt-d-x64-1_85.a"
        "imgui"
        ${tbb12}
        sfml-system
        sfml-audio
        flecs::flecs_static
        ${wx_lib}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(INSTALL ${sfml_BINARY_DIR}/lib/sfml-system-d-2.dll DESTINATION ${Target_dir})
    file(INSTALL ${sfml_BINARY_DIR}/lib/sfml-audio-d-2.dll DESTINATION ${Target_dir})
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    file(INSTALL ${sfml_BINARY_DIR}/lib/sfml-system-2.dll DESTINATION ${Target_dir})
    file(INSTALL ${sfml_BINARY_DIR}/lib/sfml-audio-2.dll DESTINATION ${Target_dir})
endif ()

#add_custom_command(TARGET mylib POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:mylib> ${CMAKE_BINARY_DIR}/bin/mylib.dll)


