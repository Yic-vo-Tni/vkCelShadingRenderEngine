cmake_minimum_required(VERSION 3.29)
project(HakuroFabric LANGUAGES CXX)

file(GLOB_RECURSE SRCs "engine/*.h" "engine/*.cpp")
file(GLOB_RECURSE PCHs "Engine/External/*.h")

function(hf_set_target target)
    target_compile_features(${target} PUBLIC cxx_std_23)

    target_include_directories(${target} PUBLIC
            ${CMAKE_SOURCE_DIR}/src/Engine
            ${CMAKE_SOURCE_DIR}/subprojects
    )

    target_precompile_headers(${target} PUBLIC ${PCHs})

    if (MINGW)
        target_compile_options(${target} PUBLIC
                -mavx
                -D__USE_MINGW_ANSI_STDIO=1
        )
        target_link_options(${target} PUBLIC
                -fuse-ld=lld
                -flto=thin
                $<$<CONFIG:Release>:-mwindows>
        )
    endif ()

    if (MSVC)
        target_compile_options(${target} PUBLIC
                /arch:AVX
                $<$<CONFIG:Release>:/O2>
        )

        target_link_options(${target} PUBLIC
                $<$<CONFIG:Release>:/LTCG>
#                $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
        )
    endif ()
endfunction()

find_package(Vulkan REQUIRED)

set(shader_path "${CMAKE_SOURCE_DIR}/src/resource/shaders/")
set(r_shader_path "/src/resource/shaders/")
set(spv_path "${CMAKE_SOURCE_DIR}/src/resource/shaders_spv/")
set(imgui_ini_path "${CMAKE_SOURCE_DIR}/src/Engine/Editor/")
set(webview_path "${CMAKE_SOURCE_DIR}/src/resource/webview/")
set(tex_path "${CMAKE_SOURCE_DIR}/src/resource/Texture/")

configure_file(
        "Engine/Foundation/config.h.in"
        "${CMAKE_SOURCE_DIR}/src/Engine/Foundation/config.h"
)

#----------------------------HakuroFabric::HakuroFabric-------------------------------------

add_library(HakuroFabric STATIC ${SRCs} "Engine/Runtime/Model/common_device.cu")
add_library(HakuroFabric::HakuroFabric ALIAS HakuroFabric)
set_target_properties(HakuroFabric PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
hf_set_target(HakuroFabric)
target_link_libraries(HakuroFabric PUBLIC
        assimp
        glfw
        webview::core
        HF_HEADER::entt
        HF_HEADER::json
        HF_HEADER::stb
        spdlog::spdlog_header_only
        glm::glm-header-only
        Saba
        miniaudio
        mimalloc-static
        Vulkan::Vulkan
        imgui
        ws2_32
)


#----------------------------HakuroFabric::RenderFlow-------------------------------------

add_library(RenderFlow SHARED "Engine/Runtime/System/RenderStage/AlgorithmNode/Forward.cpp")
add_library(HakuroFabric::RenderFlow ALIAS RenderFlow)
set_target_properties(RenderFlow PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
hf_set_target(RenderFlow)
target_link_libraries(RenderFlow PUBLIC
        HakuroFabric

        assimp
        glfw
        webview::core
        HF_HEADER::entt
        HF_HEADER::json
        HF_HEADER::stb
        spdlog::spdlog_header_only
        glm::glm-header-only
        Saba
        miniaudio
        mimalloc-static
        Vulkan::Vulkan
        imgui
        ws2_32
)



















