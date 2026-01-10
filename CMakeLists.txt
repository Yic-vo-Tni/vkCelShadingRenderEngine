cmake_minimum_required(VERSION 3.29)

project(VkCelShadingRenderer LANGUAGES CXX )

#add_compile_options(-UTBB_USE_DEBUG)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG -fsanitize=address,undefined")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG ")

###############
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
# oneTbb
#set(TBB_TEST OFF CACHE BOOL "" FORCE)
#set(TBB_EXAMPLES OFF CACHE BOOL "" FORCE)
#set(TBB_STRICT OFF CACHE BOOL "" FORCE)
#set(TBB_WARNING_AS_ERROR OFF CACHE BOOL "" FORCE)

if (MSVC)
    enable_language(CUDA)
    set(CUDAToolkit_ROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6")
    set(CMAKE_CUDA_COMPILER "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6/bin/nvcc.exe")
    set(CMAKE_CUDA_ARCHITECTURES 86)
    set(CMAKE_CUDA_STANDARD 23)
endif ()

set(CMAKE_CXX_STANDARD 23)
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX /utf-8")
#    add_compile_options("/utf-8")
elseif (MINGW)
    #set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -flto=thin")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld -flto=thin")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -D__USE_MINGW_ANSI_STDIO=1")
endif ()

add_executable(VkCelShadingRenderer main.cpp)


target_link_options(VkCelShadingRenderer PRIVATE $<$<CONFIG:Release>:-mwindows>)
set_target_properties(VkCelShadingRenderer PROPERTIES
        LINK_FLAGS_RELEASE "-mwindows"
)



add_subdirectory(src/external/assimp)
add_subdirectory(src/external/glfw)

#add_compile_options(-Wno-unused-command-line-argument)
#set(TBB_USE_DEBUG OFF CACHE BOOL "" FORCE)
#set(TBB_ENABLE_ASSERTIONS OFF CACHE BOOL "" FORCE)
#set(TBB_STRICT OFF CACHE BOOL "" FORCE)
#set(TBB_ENABLE_ITT OFF CACHE BOOL "" FORCE)
#
#set(_OLD_BUILD_TYPE ${CMAKE_BUILD_TYPE})
#set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
#
#add_subdirectory(src/external/oneTBB)
#set(CMAKE_BUILD_TYPE ${_OLD_BUILD_TYPE} CACHE STRING "" FORCE)

#Saba
set(SABA_ENABLE_TEST OFF CACHE BOOL "" FORCE)
set(SABA_FORCE_GLFW_BUILD OFF CACHE BOOL "" FORCE)
add_subdirectory(subprojects)
add_subdirectory(src)

#tbb
#target_compile_options(renderer PRIVATE -UTBB_USE_DEBUG)

target_link_libraries(VkCelShadingRenderer PRIVATE renderer)
