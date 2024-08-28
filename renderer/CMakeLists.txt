cmake_minimum_required(VERSION 3.28)
project(Renderer)

set(BOOST_ROOT "F:/Framework/boost_1_85_0")
set(BOOST_LIBRARYDIR  "${BOOST_ROOT}/stage/lib")
message("Boost root: ${BOOST_ROOT}")
message("Boost library directory: ${BOOST_LIBRARYDIR}")

file(GLOB_RECURSE SRCS "src/*.h" "src/*.cpp")
file(GLOB wx_lib "../external/wxWidget/*.a")
#file(GLOB_RECURSE bt_lib "../external/Bullet/*.a")
find_library(mimalloc mimalloc-static "../external/mimalloc")
#find_library(saba Saba PATHS "../external/Saba")

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
        nfd
        GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
        GIT_TAG master
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/nfd"
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

FetchContent_GetProperties(nfd)
if (NOT nfd_POPULATED)
    FetchContent_Populate(nfd)
#    add_subdirectory(${nfd_SOURCE_DIR} ${nfd_BINARY_DIR})
#    link_directories(${nfd_BINARY_DIR}/lib)
endif ()

project(nfd)
file(GLOB_RECURSE nfd_srcs "third/nfd/src/*.cpp" "third/nfd/src/*.h" "third/nfd/src/nfd_common.c" "third/nfd/src/nfd_win.cpp")
add_library(nfd STATIC ${nfd_srcs})

add_library(Renderer STATIC ${SRCS} )
target_precompile_headers(Renderer PUBLIC ${pch_files})
target_include_directories(Renderer PUBLIC
        ../external
        third
        third/spdlog
        src
        ../subprojects/ImGui
        ../subprojects/saba/src
        third/sfml/include
        third/wxWidget
        third/nfd/src/include
        third/mimalloc/include
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
#        ${saba}
        "Saba"
        nfd
        ${mimalloc}
#        ${bt_lib}
       # BulletDynamics BulletCollision BulletSoftBody LinearMath
)

file(GLOB_RECURSE DLL_FILES
        "${sfml_BINARY_DIR}/*.dll"
        "${assimp_BINARY_DIR}/*.dll"
        "${CMAKE_BINARY_DIR}/subprojects/*.dll"
)

    message("dll files found: ${DLL_FILES}")
foreach (dll_file IN LISTS DLL_FILES)
    message("Installing: ${dll_file}")
    message("binary dir: ${CMAKE_BINARY_DIR}")
    file(INSTALL ${dll_file} DESTINATION ${CMAKE_BINARY_DIR})
endforeach ()

add_custom_target(CopyDLLs ALL)
add_custom_command(
        TARGET CopyDLLs POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying DLL files after build..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${sfml_BINARY_DIR}/*.dll" "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${assimp_BINARY_DIR}/*.dll" "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/subprojects/*.dll" "${CMAKE_BINARY_DIR}"
        COMMENT "DLL files are copied to the binary directory."
)





