cmake_minimum_required(VERSION 3.29)
project(renderer LANGUAGES CXX)

set(IS_CLANG FALSE)
set(IS_GNU FALSE)
set(IS_MSVC FALSE)
set(IS_LIBCXX FALSE)

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-stdlib=libc++")
check_cxx_source_compiles("
#include <string>
int main(){ std::string s; return 0; }
" IS_LIBCXX)
unset(CMAKE_REQUIRED_FLAGS)

message(STATUS "Compiler ID      : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler Target  : ${CMAKE_CXX_COMPILER_TARGET}")
message(STATUS "Using libc++     : ${IS_LIBCXX}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(IS_CLANG TRUE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(IS_GNU TRUE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(IS_MSVC TRUE)
endif ()

#if (IS_MSVC)
#    file(GLOB_RECURSE THIRD_LIBS "third/lib/msvc/*.lib")
#elseif (IS_CLANG AND IS_LIBCXX)
#    file(GLOB_RECURSE THIRD_LIBS "third/lib/ucrt_clang/*.a")
#elseif (IS_CLANG OR IS_GNU)
#    file(GLOB_RECURSE THIRD_LIBS "third/lib/mingw/*.a")
#endif ()


if (MSVC)
    enable_language(CUDA)
    file(GLOB_RECURSE SRCs "engine/*.h" "engine/*.cpp" "Engine/*.cuh" "Engine/*.cu")
elseif (MINGW)
    file(GLOB_RECURSE SRCs "engine/*.h" "engine/*.cpp" )
endif ()
#file(GLOB_RECURSE Libs "third/lib/*.lib")
#file(GLOB_RECURSE Dlls "third/dll/*.dll")
file(GLOB_RECURSE MSVC_Libs "third/lib/msvc/*.lib")
file(GLOB_RECURSE MinGW_Libs "third/lib/mingw/*.a")
file(GLOB_RECURSE MSVC_DLLs "third/dll/msvc/*.dll")
file(GLOB_RECURSE MinGW_DLLs "third/dll/mingw/*.dll")
file(GLOB_RECURSE PCHs "Engine/External/*.h")

add_library(Compose SHARED "Engine/Runtime/System/RenderStage/AlgorithmNode/Forward.cpp")
add_library(renderer STATIC ${SRCs} "Engine/Runtime/Model/common_device.cu")

#target_compile_options(renderer PRIVATE /MDd)

find_package(Vulkan REQUIRED)
find_package(CUDAToolkit REQUIRED)

target_precompile_headers(Compose PUBLIC ${PCHs})
target_precompile_headers(renderer PUBLIC ${PCHs})

set(shader_path "${CMAKE_SOURCE_DIR}/src/resource/shaders/")
set(r_shader_path "/src/resource/shaders/")
set(spv_path "${CMAKE_SOURCE_DIR}/src/resource/shaders_spv/")
set(imgui_ini_path "${CMAKE_SOURCE_DIR}/src/Engine/Editor/")
set(webview_path "${CMAKE_SOURCE_DIR}/src/resource/webview/")
set(tex_path "${CMAKE_SOURCE_DIR}/src/resource/Texture/")
set(common_libs
        Vulkan::Vulkan
        imgui
        webview::core
        Saba
        ws2_32
)
configure_file("Engine/External/config.h.in" "${CMAKE_SOURCE_DIR}/src/Engine/External/config.h")

target_include_directories(Compose PUBLIC
        engine
        third
        ../subprojects
        ${CUDAToolkit_INCLUDE_DIRS}
        ../subprojects/saba/src
        third/flecs/include
)
target_include_directories(renderer PUBLIC
        engine
        third
        ../subprojects
        ${CUDAToolkit_INCLUDE_DIRS}
        ../subprojects/saba/src
        third/flecs/include
)

if (MSVC)
    target_link_libraries(renderer PUBLIC
            ${MSVC_Libs}
            CUDA::cudart
            ${common_libs}
    )

    foreach (dll_file IN LISTS MSVC_DLLs)
        message("Installing: ${MSVC_DLLs}")
        message("binary dir: ${CMAKE_BINARY_DIR}")
        file(INSTALL ${dll_file} DESTINATION ${CMAKE_BINARY_DIR})
    endforeach ()
elseif (MINGW)
    target_link_libraries(Compose PUBLIC
            ${MinGW_Libs}
            ${common_libs}
            iconv

            assimp
            glfw
            renderer
    )

    target_link_libraries(renderer PUBLIC
            ${MinGW_Libs}
            ${common_libs}
            iconv

            assimp
            glfw
    )

    foreach (dll_file IN LISTS MinGW_DLLs)
        message("Installing: ${MinGW_DLLs}")
        message("binary dir: ${CMAKE_BINARY_DIR}")
        file(INSTALL ${dll_file} DESTINATION ${CMAKE_BINARY_DIR}/bin)
    endforeach ()
endif ()

set_target_properties(renderer PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
)

file(INSTALL ../subprojects/saba/viewer/Saba/Viewer/resource DESTINATION ${CMAKE_BINARY_DIR}/bin)






