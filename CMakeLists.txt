cmake_minimum_required(VERSION 3.29)

project(HF LANGUAGES CXX )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)

    add_compile_options(
            -Wno-unsafe-buffer-usage
            -Wno-switch-default
            -Wno-format
            -Wno-unreachable-code-break
            -Wno-unused-member-function
            -Wno-unused-parameter
            -Wno-unused-variable
            -Wno-shadow
            -Wno-sign-conversion
            -Wno-implicit-int-float-conversion
            -Wno-old-style-cast
            -Wno-covered-switch-default
            -Wno-implicit-fallthrough
            -Wno-extra-semi
    )
endif()

include(cmake/ThirdParty.cmake)
#Saba
set(SABA_ENABLE_TEST OFF CACHE BOOL "" FORCE)
set(SABA_FORCE_GLFW_BUILD OFF CACHE BOOL "" FORCE)
add_subdirectory(subprojects)

add_executable(HF main.cpp)

set_target_properties(HF PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_subdirectory(src)

target_link_libraries(HF PRIVATE HakuroFabric::HakuroFabric)

add_custom_target(HF_runtime ALL
        DEPENDS HF
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:HF>
)

#if (MINGW)
#    add_custom_command(TARGET HF_runtime POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            $<TARGET_FILE:glfw>
#            $<TARGET_FILE:assimp>
#            $<TARGET_FILE:spdlog>
#            $<TARGET_FILE_DIR:HF>
#            VERBATIM
#    )
#endif()

#if (MSVC)
#    add_custom_command(TARGET HF_runtime POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            $<TARGET_RUNTIME_DLLS:HF>
#            $<TARGET_FILE_DIR:HF>
#            COMMAND_EXPAND_LISTS
#            VERBATIM
#    )
#endif()