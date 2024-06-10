# CheckShader.cmake
file(TIMESTAMP ${src} src_time)
file(TIMESTAMP ${dst} dst_time)

# 假设如果没有时间戳，就需要编译
if(NOT src_time STREQUAL "" AND (dst_time STREQUAL "" OR src_time GREATER dst_time))
    file(REMOVE ${dst})
endif()
