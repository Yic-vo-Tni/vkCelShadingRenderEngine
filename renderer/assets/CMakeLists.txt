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

#foreach(shader ${shaders})
#    get_filename_component(shader_name ${shader} NAME_WE)
#    set(output_spv "${spv}/${shader_name}.spv")
#    add_custom_command(
#            OUTPUT ${output_spv}
#            COMMAND ${CMAKE_COMMAND} -Dsrc=${shader} -Ddst=${output_spv} -P ${CMAKE_SOURCE_DIR}/CheckShader.cmake
#            COMMAND ${glsl} ${shader} --target-env=vulkan1.3 -o ${output_spv}
#            DEPENDS ${shader}
#            COMMENT "Potentially recompiling ${shader_name}"
#    )
#endforeach()
#
#add_custom_target(shaders ALL DEPENDS ${output_spv})