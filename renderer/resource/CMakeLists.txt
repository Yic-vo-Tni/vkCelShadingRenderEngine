cmake_minimum_required(VERSION 3.28)
project(shader)

find_program(glsl glslc REQUIRED)

set(src "${CMAKE_SOURCE_DIR}/shaders")
set(spv "${CMAKE_SOURCE_DIR}/shaders_spv")

file(GLOB_RECURSE  shaders CONFIGURE_DEPENDS
        "${src}/*.vert"
        "${src}/*.frag"
        "${src}/*.geom"
        "${src}/*.rgen"
        "${src}/*.rchit"
        "${src}/*.rmiss"
        "${src}/*.comp"
)

foreach (shader ${shaders})
    get_filename_component(shader_name ${shader} NAME)
    get_filename_component(shader_path ${shader} DIRECTORY)
    file(RELATIVE_PATH shader_rel_path ${src} ${shader_path})
    set(output_path "${spv}/${shader_rel_path}")
    file(MAKE_DIRECTORY ${output_path})

    execute_process(COMMAND ${glsl} ${shader} --target-env=vulkan1.3 -o ${output_path}/${shader_name}.spv)
endforeach ()

#set(SPV_OUTPUTS "")
#
#foreach(SHADER ${SHADERS})
#    get_filename_component(FILE_NAME ${SHADER} NAME)
#    get_filename_component(FILE_DIR  ${SHADER} DIRECTORY)
#    file(RELATIVE_PATH REL_PATH ${SRC} ${FILE_DIR})
#    set(OUT_DIR "${SPV}/${REL_PATH}")
#    file(MAKE_DIRECTORY ${OUT_DIR})
#
#    set(OUT_FILE "${OUT_DIR}/${FILE_NAME}.spv")
#
#    add_custom_command(
#            OUTPUT ${OUT_FILE}
#            COMMAND ${GLSLC} ${SHADER} --target-env=vulkan1.3 -o ${OUT_FILE}
#            DEPENDS ${SHADER}
#            BYPRODUCTS ${OUT_FILE}
#            COMMENT "Compiling shader ${SHADER} -> ${OUT_FILE}"
#            VERBATIM
#    )
#
#    list(APPEND SPV_OUTPUTS ${OUT_FILE})
#endforeach()
#
#add_custom_target(compile_shaders ALL DEPENDS ${SPV_OUTPUTS})
#add_dependencies(VkCelShadingRenderer compile_shaders)