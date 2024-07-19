//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_PCH_H
#define VKCELSHADINGRENDERER_PCH_H

//#define WIN32_LEAN_AND_MEAN
#include "winsock2.h"
#include "windows.h"

#include "intrin.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.hpp"

#include "vma/vk_mem_alloc.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb/stb_image.h"

#include "iostream"
#include "stdexcept"
#include "vector"
#include "set"
#include "optional"
#include "limits"
#include "utility"
#include "chrono"
#include "memory"
#include "fstream"
#include "bitset"
#include "unordered_map"
#include "variant"
#include "tuple"
#include "thread"
#include "mutex"
#include "functional"
#include "map"
#include "queue"
#include "random"
#include "sstream"
#include "future"
#include "concepts"
#include "typeinfo"
#include "typeindex"
#include "type_traits"
#include "ranges"
#include "stack"
#include "any"
#include "filesystem"
#include "semaphore"

#include "include/spdlog/spdlog.h"
#include "include/spdlog/sinks/stdout_color_sinks.h"
#include "include/spdlog/fmt/ostr.h"
#include "include/spdlog/sinks/ansicolor_sink.h"

#include "boost/hana.hpp"
#include "boost/process.hpp"
#include "boost/preprocessor.hpp"
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/ordered_index.hpp"

#include "oneapi/tbb/task_group.h"
#include "oneapi/tbb/parallel_invoke.h"
#include "oneapi/tbb/concurrent_map.h"
#include "oneapi/tbb/concurrent_unordered_map.h"
#include "oneapi/tbb/spin_rw_mutex.h"
#include "oneapi/tbb/queuing_rw_mutex.h"

#include "magic_enum/include/magic_enum/magic_enum.hpp"

#include "flecs/flecs.h"

#include "wx/wx.h"
#include "wx/stc/stc.h"
#include "wx/file.h"
#include "wx/button.h"
#include "wx/aui/auibook.h"

#ifdef NDEBUG
const bool enableDebug = false;
#else
const bool enableDebug = true;
#endif

#define if_debug  if(enableDebug)



#endif //VKCELSHADINGRENDERER_PCH_H
