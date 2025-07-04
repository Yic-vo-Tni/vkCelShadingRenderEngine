//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_PCH_H
#define VKCELSHADINGRENDERER_PCH_H

#include "winsock2.h"
#include "windows.h"

#include "intrin.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.hpp"

#include "vma/vk_mem_alloc.h"

#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"
#include "ImGui/imgui_internal.h"

#include "ImGui/ImGuizmo.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/gtx/string_cast.hpp"

#include "cuda_runtime.h"

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
#include "list"
#include "unordered_set"
#include <regex>
#include "immintrin.h"
#include "memory_resource"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/ansicolor_sink.h"

#include "boost/hana.hpp"
#include "boost/locale.hpp"
#include "boost/process.hpp"
#include "boost/pool/object_pool.hpp"
#include "boost/pool/pool.hpp"

#include "oneapi/tbb/task_group.h"
#include "oneapi/tbb/parallel_invoke.h"
#include "oneapi/tbb/concurrent_map.h"
#include "oneapi/tbb/concurrent_unordered_map.h"
#include "oneapi/tbb/spin_rw_mutex.h"
#include "oneapi/tbb/queuing_rw_mutex.h"
#include "oneapi/tbb/parallel_for_each.h"
#include "oneapi/tbb/concurrent_queue.h"
#include "oneapi/tbb/global_control.h"
#include "oneapi/tbb/mutex.h"
#include "oneapi/tbb/combinable.h"

//#include "flecs/include/flecs.h"
#include "entt/entt.hpp"

#include "mimalloc/include/mimalloc.h"


/// forward
namespace rhi{ class Descriptor; }
using DescriptorInterface = rhi::Descriptor;

namespace vot::inline rhi{

    struct Buffer;
    struct Image;
    struct Accel;
    using Buffer_sptr = std::shared_ptr<Buffer>;
    using Image_sptr = std::shared_ptr<Image>;
    using Accel_sptr = std::shared_ptr<Accel>;
    using Buffer_wptr = std::weak_ptr<Buffer>;
    using Image_wptr = std::weak_ptr<Image>;
    using Accel_wptr = std::weak_ptr<Accel>;

    using Descriptor_sptr = std::shared_ptr<DescriptorInterface>;
}

#define SS(Type, Property, CamelCaseProperty) \
    Type Property; \
    auto& set##CamelCaseProperty(const Type& value) { (Property) = value; return *this; }

#define SS_OPT(Type, Property, CamelCaseProperty) \
    std::optional<Type> Property; \
    auto& set##CamelCaseProperty(const Type& value) { (Property) = value; return *this; }
#endif //VKCELSHADINGRENDERER_PCH_H
