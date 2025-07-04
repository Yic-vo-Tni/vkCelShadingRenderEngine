cmake_minimum_required(VERSION 3.29)
project(renderer LANGUAGES CXX)

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

add_library(renderer STATIC ${SRCs} "Engine/ECS/Model/common_device.cu")

#target_compile_options(renderer PRIVATE /MDd)

find_package(Vulkan REQUIRED)
find_package(CUDAToolkit REQUIRED)

target_precompile_headers(renderer PUBLIC ${PCHs})

set(shader_path "${CMAKE_SOURCE_DIR}/renderer/resource/shaders/")
set(r_shader_path "/renderer/resource/shaders/")
set(spv_path "${CMAKE_SOURCE_DIR}/renderer/resource/shaders_spv/")
set(imgui_ini_path "${CMAKE_SOURCE_DIR}/renderer/Engine/Editor/")
set(webview_path "${CMAKE_SOURCE_DIR}/renderer/resource/webview/")
set(common_libs
        Vulkan::Vulkan
        imgui
        webview::core
        Saba
        ws2_32
)
configure_file("Engine/External/config.h.in" "${CMAKE_SOURCE_DIR}/renderer/Engine/External/config.h")


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
    target_link_libraries(renderer PUBLIC
            ${MinGW_Libs}
            ${common_libs}
            iconv
    )

    foreach (dll_file IN LISTS MinGW_DLLs)
        message("Installing: ${MinGW_DLLs}")
        message("binary dir: ${CMAKE_BINARY_DIR}")
        file(INSTALL ${dll_file} DESTINATION ${CMAKE_BINARY_DIR})
    endforeach ()
endif ()

set_target_properties(renderer PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
)

file(INSTALL ../subprojects/saba/viewer/Saba/Viewer/resource DESTINATION ${CMAKE_BINARY_DIR})





