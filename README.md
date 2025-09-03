# vkCelShadingRenderEngine

> Polished English version by ChatGPT.

[简体中文](README.zh-CN.md)

## Overview

- This repository is a personal record of self-learning and engineering practice, mainly for learning and personal use.
  Any suggestions or feedback are welcome!
- Mainly used for exploring and learning Vulkan by myself. All comments and advice are appreciated. (╹▽╹)

### Screenshots

<img src="screenShot/01.png" width="340"/> <img src="screenShot/02.png" width="340"/>

### Learning Goals & Planned Features

> *Note: The following features are goals or planned work. Most are still in progress and not yet available in the
current codebase.*

**vkCelShadingRenderEngine = Architecture * Atmosphere * Elegance + Bonus**

---

- **System Graph**

  Unified management and scheduling of global systems, including subsystems such as the render graph.
- **Advanced Vulkan Extensions**

  GPU-side parallel command buffer construction.
- **High-End Rendering Effects**

  Voxel grid volumetric fog/clouds and physically-based precomputed atmosphere.
- **Animation System**

  Deconstructing Saba library for future experiments in VR, physics, and cloth simulation.
- **Optimization && AI**

### Current Project Status

> Each item is marked with one of the following statuses:
>
> - ✔️  A basic implementation exists and is functional (may not be fully complete).
> - 📋  Work in progress: partially implemented or planned but not fully realized.
> - ❌  Deprecated or abandoned: feature has been removed or is no longer supported.
>
> This section reflects the **actual progress and core features** of the engine,
> rather than long-term aspirations or experimental prototypes.

| Engine Architecture | Status | Notes     / Progress                                                  | Refactor / Plan                            |
|---------------------|:------:|-----------------------------------------------------------------------|--------------------------------------------|
| Event               |   ✔️   | Global pub/sub, async, unique events                                  | Sub-event mounting on main event, sync     |
| Task                |   📋   |                                                                       | Global task execution via tbb_graph        |
| Storage             |   ✔️   | Thread-safe storage, type-safe opt                                    | Refactor into central storage, remove HANA |
| ECS                 |   ✔️   | EnTT-based, modular                                                   |                                            |
| Threads             |   ✔️   | Window / fast logic / slow logic / render threads, triple-buffer sync | Further split render thread, decouple cmd  |
| Animation           |   ✔️   | Assimp+Saba, skin+VMD, parallel multi-model playback                  | Deconstruct Saba, move data into ECS       |
| Audio               |   ✔️   | miniaudio, music playback & pause                                     |                                            |
| Material            |   📋   |                                                                       |                                            |
| Physical            |   📋   |                                                                       |                                            |
| Scene               |   ✔️   | Scene management, global TLAS                                         | Dynamic TLAS / Static TLAS                 |
| Light               |   📋   |                                                                       | Add point light support                    |

| Rendering System   | Status | Notes / Progress                            | Refactor / Plan                     |
|--------------------|:------:|---------------------------------------------|-------------------------------------|
| Dynamic Rendering  |   ✔️   | VK_KHR_dynamic_rendering, no framebuffer    |                                     |
| Render Graph       |   ✔️   | Basic graph, auto dependency resolution     | Async scheduling, GPU timing        |
| Deferred Rendering |   ✔️   | G-buffer pipeline                           |                                     |
| Sync (Semaphore)   |   ✔️   | Timeline semaphore, VK_KHR_synchronization2 |                                     |
| Descriptor System  |   ✔️   | Global set0, bindless ready                 | Migrate to VK_EXT_descriptor_buffer |
| Pipeline Library   |   ✔️   | Precompiled pipelines, dynamic switching    |                                     |
| Allocator          |   ✔️   | VMA-based, LRU allocator                    | Three-pool, lighter interface       |
| Command System     |   ✔️   | Thread-local cmd pools, efficient recycling | Parallel build path                 |
| RT Shadows         |   ✔️   | VK_KHR_ray_tracing, dynamic BLAS            |                                     |
| Shadow Map         |   ❌    |                                             |                                     |
| Volumetric Clouds  |   ✔️   | Ray-marching, Shadertoy migrated            | Learn precomputed atmosphere        |
| Volumetric Fog     |   ✔️   | Ray-marching, screen-space fog              | Semi-voxel grid volumetric fog      |
| HDR                |   📋   |                                             |                                     |
| Mesh Shader        |   📋   | VK_EXT_mesh_shader planned                  |                                     |


| Resource Systems       | Status | Notes / Progress                          | Refactor / Plan                          |
|------------------------|:------:|-------------------------------------------|------------------------------------------|
| Async Resource Loading |   ✔️   | Async loading, avoid main thread, barrier | Parallelization                          |
| Standard Model         |   ✔️   | Assimp loading, std::pmr::vector          | Combine pmr with mimalloc                |
| Standard Images        |   ✔️   | stb_image loading                         | Split Image class, move part to RT       |
| MMD Support            |   ✔️   | Saba-based implementation                 | Same refactor plan as Assimp Model       |
| Chinese Path Support   |   ✔️   | Boost_locale for Windows path issues      | Remove locale (over-engineered solution) |

| Toolchain / Editor Systems  | Status | Notes / Progress                                              | Refactor / Plan      |
|-----------------------------|:------:|---------------------------------------------------------------|----------------------|
| Shader Hot Reload           |   ✔️   | Monaco-editor + webview, Ctrl+S hot compile                   | VK_EXT_shader_object |
| Window Drag & Docking       |   ✔️   | ImGui docking                                                 |                      |
| Model Basic Manipulation    |   ✔️   | ImGuizmo support                                              |                      |
| Model Selection (ID buffer) |   ✔️   | Render ID buffer, mouse mapping via ImGui, EnTT entity ID map |                      |
| Debug Output                |   📋   | Planned Vulkan debug output / GPU markers                     |                      |

| Vulkan Extension / Feature            | Status | Refactor / Plan |
|---------------------------------------|:------:|-----------------|
| VK_LAYER_KHRONOS_validation           |   ✔️   |                 |
| VK_EXT_debug_utils                    |   ✔️   |                 |
| ShaderInt64 (feature)                 |   📋   |                 |
| SamplerAnisotropy (feature)           |   📋   |                 |
| GeometryShader (feature)              |   📋   |                 |
| RobustBufferAccess (feature)          |   📋   |                 |
| TessellationShader (feature)          |   📋   |                 |
| VK_KHR_swapchain                      |   ✔️   |                 |
| VK_KHR_deferred_host_operations       |   📋   |                 |
| VK_KHR_spirv_1_4                      |   📋   |                 |
| VK_KHR_create_renderpass2             |   ❌    |                 |
| VK_KHR_pipeline_library               |   ✔️   |                 |
| VK_KHR_shader_non_semantic_info       |   📋   |                 |
| VK_EXT_pipeline_creation_feedback     |   ✔️   |                 |
| VK_KHR_ray_tracing_pipeline           |   ✔️   |                 |
| VK_KHR_acceleration_structure         |   ✔️   |                 |
| VK_KHR_buffer_device_address          |   ✔️   |                 |
| VK_KHR_synchronization2               |   ✔️   |                 |
| VK_KHR_timeline_semaphore             |   ✔️   |                 |
| VK_KHR_dynamic_rendering              |   ✔️   |                 |
| VK_KHR_pipeline_executable_properties |   📋   |                 |
| VK_EXT_shader_object                  |   📋   |                 |
| VK_EXT_descriptor_indexing            |   ✔️   |                 |
| VK_EXT_descriptor_buffer              |   📋   |                 |
| VK_EXT_transform_feedback             |   📋   |                 |
| VK_EXT_conditional_rendering          |   📋   |                 |
| VK_EXT_graphics_pipeline_library      |   ✔️   |                 |
| VK_EXT_shader_module_identifier       |   📋   |                 |
| VK_NVX_multiview_per_view_attributes  |   📋   |                 |
| VK_NV_device_generated_commands       |   📋   |                 |
| VK_EXT_mesh_shader                    |   📋   |                 |
| VK_KHR_dynamic_rendering_local_read   |   ✔️   |                 |
| VK_EXT_robustness2                    |   📋   |                 |

| Third-party Libraries  | ✔️ | 🚧 | 📋 | ❌ | Refactor / Plan  |
|------------------------|:--:|:--:|:--:|:-:|------------------|
| assimp                 | ✔️ |    |    |   |                  |
| boost                  | ✔️ |    |    |   | Planned removal  |
| entt                   | ✔️ |    |    |   |                  |
| flecs                  |    |    |    | ❌ |                  |
| glfw                   | ✔️ |    |    |   |                  |
| glm                    | ✔️ |    |    |   |                  |
| mimalloc               | ✔️ |    |    |   |                  |
| miniaudio              | ✔️ |    |    |   |                  |
| nlohmann               | ✔️ |    |    |   |                  |
| oneapi                 | ✔️ |    |    |   |                  |
| ozz                    |    |    | 📋 |   |                  |
| spdlog                 | ✔️ |    |    |   |                  |
| stb                    | ✔️ |    |    |   |                  |
| webview                | ✔️ |    |    |   |                  |
| saba                   | ✔️ |    |    |   | Being refactored |
| imgui-docking/imguizmo | ✔️ |    |    |   |                  |
| cuda                   |    |    | 📋 |   |                  |
| bullet                 |    |    |    | ❌ |                  |
| vma                    | ✔️ |    |    |   |                  |
| jolt physics           |    |    | 📋 |   |                  |


### Legacy / Deprecated Features
> The following features were implemented in earlier stages of the engine but have since been removed or replaced.  
> They are kept here as a record of experimentation and evolution.

| Legacy Feature              | Notes / Reason for Removal                     |
|-----------------------------|------------------------------------------------|
| Secondary Cmd Parallel      | Rewritten, replaced with improved design       |
| ShadowMap + PCF             | Replaced by real-time ray traced shadows       |
| Skybox (cube map)           | Legacy, replaced by volumetric atmosphere      |
| Simple Volumetric Fog/Noise | FastNoiseLite, replaced by full volumetric fog |
| wx_widget Shader Reload     | Replaced by Monaco-editor + webview solution   |
| 120fps Window Lock          | Removed, unnecessary with new thread system    |
| Model Selection (Ray pick)  | Replaced by ID buffer-based picking            |

## How to Build
The project will be reorganized with **Git submodules** (for third-party dependencies) after:
- Removing the Boost library
- Refactoring the Saba library  

