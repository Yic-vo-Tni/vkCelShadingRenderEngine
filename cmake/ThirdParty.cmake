set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(_FETCH_DEPS_DIR "${CMAKE_BINARY_DIR}/_deps")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
        AND MINGW
        AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Using shared FetchContent deps for MinGW-clang-Debug")
    set(_FETCH_DEPS_DIR "${CMAKE_SOURCE_DIR}/.deps")
endif()

set(FETCHCONTENT_BASE_DIR "${_FETCH_DEPS_DIR}")

include(FetchContent)

#-------------------------webview------------------------------
FetchContent_Declare(
        webview
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/webview
)

FetchContent_MakeAvailable(webview)

#-------------------------webview------------------------------
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        assimp
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/assimp
)

FetchContent_MakeAvailable(assimp)

#-------------------------glfw------------------------------
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_INCLUDE_NONE ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        glfw
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/glfw
)

FetchContent_MakeAvailable(glfw)

#-------------------------entt------------------------------
FetchContent_Declare(
        entt
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/entt
)
FetchContent_MakeAvailable(entt)
add_library(entt INTERFACE)
add_library(HF_HEADER::entt ALIAS entt)
target_include_directories(entt INTERFACE
    ${entt_SOURCE_DIR}/single_include
)

#-------------------------json------------------------------
FetchContent_Declare(
        json
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/json
)
FetchContent_MakeAvailable(json)
add_library(json INTERFACE)
add_library(HF_HEADER::json ALIAS json)
target_include_directories(json INTERFACE
    ${json_SOURCE_DIR}/single_include
)

#-------------------------spdlog------------------------------
FetchContent_Declare(
        spdlog
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/third/spdlog
)
FetchContent_MakeAvailable(spdlog)

#-------------------------glm------------------------------
FetchContent_Declare(
        glm
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/third/glm
)
FetchContent_MakeAvailable(glm)

#-------------------------saba------------------------------
FetchContent_Declare(
        saba
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/saba
)
FetchContent_MakeAvailable(saba)

#-------------------------mimalloc------------------------------
FetchContent_Declare(
        mimalloc
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/mimalloc
)
FetchContent_MakeAvailable(mimalloc)

#-------------------------miniaudio------------------------------
FetchContent_Declare(
        miniaudio
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/subprojects/miniaudio
)
FetchContent_MakeAvailable(miniaudio)

#-------------------------stb------------------------------
FetchContent_Declare(
        stb
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/third/stb
)
FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE)
add_library(HF_HEADER::stb ALIAS stb)
target_include_directories(stb INTERFACE
        ${stb_SOURCE_DIR}
)