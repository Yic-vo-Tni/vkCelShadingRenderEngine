cmake_minimum_required(VERSION 3.28)
project(shader)

find_program(glsl glslc REQUIRED)

set(src "${CMAKE_SOURCE_DIR}/shaders")
set(spv "${CMAKE_SOURCE_DIR}/shaders_spv")

file(GLOB_RECURSE shaders
        "${src}/*.vert"
        "${src}/*.frag"
        "${src}/*.geom"
        "${src}/*.rgen"
        "${src}/*.rchit"
        "${src}/*.rmiss"
)

foreach (shader ${shaders})
    get_filename_component(shader_name ${shader} NAME)
    get_filename_component(shader_path ${shader} DIRECTORY)
    file(RELATIVE_PATH shader_rel_path ${src} ${shader_path})
    set(output_path "${spv}/${shader_rel_path}")
    file(MAKE_DIRECTORY ${output_path})

    execute_process(COMMAND ${glsl} ${shader} --target-env=vulkan1.3 -o ${output_path}/${shader_name}.spv)
endforeach ()