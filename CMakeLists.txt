cmake_minimum_required(VERSION 3.29)
project(vkCelShadingRenderer)

set(CMAKE_CXX_STANDARD 23)

set(Target_dir ${CMAKE_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (WIN32)
        set(CMAKE_WIN32_EXECUTABLE ON)
    endif ()
endif ()

set(shader_path "${CMAKE_SOURCE_DIR}/renderer/assets/shaders")
set(spv_path "${CMAKE_SOURCE_DIR}/renderer/assets/shaders_spvs")
set(imgui_ini_path "${CMAKE_SOURCE_DIR}/renderer/src/Editor/Gui")
set(img_path "${CMAKE_SOURCE_DIR}/renderer/assets/textures")
set(r_shader_path "./assets/shaders")
set(r_spv_path "./assets/shaders_spvs")
set(r_imgui_ini_path "./Gui")
set(r_img_path "./assets/textures")
configure_file("${CMAKE_SOURCE_DIR}/pch/config.h.in" "${CMAKE_SOURCE_DIR}/pch/config.h")

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    add_definitions(-DDEBUG_RESOURCE_PATH="${CMAKE_SOURCE_DIR}/renderer/assets")
#else ()
#    add_definitions(-DRESOURCE_PATH="${CMAKE_BINARY_DIR}/assets")
#endif ()

find_package(Vulkan REQUIRED)
find_program(glsl glslc REQUIRED)
find_library(glfw_msvc glfw3 PATHS external/GLFW/msvc)
find_library(glfw_mingw glfw3 PATHS external/GLFW/mingw)
find_library(tbb12 tbb12 PATHS external/tbb)

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

file(INSTALL external/tbb/libtbb12.dll DESTINATION ${CMAKE_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    file(INSTALL renderer/assets DESTINATION ${CMAKE_BINARY_DIR})
    file(INSTALL renderer/src/Editor/Guiimgui.ini DESTINATION ${CMAKE_BINARY_DIR})
endif ()

add_subdirectory(renderer)

add_subdirectory(subprojects)

include(InstallRequiredSystemLibraries)
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VENDOR "YourCompanyName")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "vkCelShadingRenderer - A Vulkan based cel shading renderer")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "vkCelShadingRenderer")
#set(CPACK_INNO_SETUP_COMPILER "D:/Software/Inno Setup 6/ISCC.exe")
set(CPACK_GENERATOR "INNOSETUP")
include(CPack)