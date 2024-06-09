cmake_minimum_required(VERSION 3.28)
project(assets)

find_program(glsl glslc REQUIRED)

set(src "${CMAKE_SOURCE_DIR}/shaders")
set(spv "${CMAKE_SOURCE_DIR}/shaders_spvs")

file(GLOB_RECURSE shaders
        "${src}/*.vert"
        "${src}/*.frag"
        "${src}/*.rgen"
        "${src}/*.rchit"
        "${src}/*.rmiss"
)

foreach (shader ${shaders})
    get_filename_component(shader_name ${shader} NAME_WE)
    execute_process(COMMAND ${glsl} ${shader} --target-env=vulkan1.3 -o ${spv}/${shader_name}.spv)
endforeach ()