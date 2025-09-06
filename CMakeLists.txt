cmake_minimum_required(VERSION 3.29)

project(VkCelShadingRenderer LANGUAGES CXX )

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG -fsanitize=address,undefined")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG ")

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


add_subdirectory(subprojects)
add_subdirectory(src)

target_link_libraries(VkCelShadingRenderer PRIVATE renderer)
