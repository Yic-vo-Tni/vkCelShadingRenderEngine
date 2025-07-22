# vkCelShadingRenderEngine

[简体中文](README.zh-CN.md)
> Polished English version by ChatGPT. 

## Overview
- This repository is a personal record of self-learning and engineering practice, mainly for learning and personal use. Any suggestions or feedback are welcome!
- Mainly used for exploring and learning Vulkan by myself. All comments and advice are appreciated. (╹▽╹)

### Screenshots
<img src="screenShot/01.png" width="340"/> <img src="screenShot/02.png" width="340"/>

### Learning Goals & Planned Features
> *Note: The following features are goals or planned work. Most are still in progress and not yet available in the current codebase.*

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
> The tables below summarize all core systems and features of the project, including those already implemented (✔️), planned (📋), and deprecated (❌).  
> Most features with a ✔️ status are already available in the current codebase and have passed initial engineering validation.  
> Items marked 📋 or 🚧 represent ongoing work or future plans.  
> This section reflects the **actual progress and core features** of the engine, rather than long-term aspirations.

| System            | Status | Notes                                |
|-------------------|:------:|--------------------------------------|
| Event             |   ✔️   | Global pub/sub, async, unique events |
| Global TS Storage |   ✔️   | Thread-safe storage, type-safe opt   |
| ECS               |   ✔️   | EnTT-based, modular                  |
| Animation         |   ✔️   | Assimp+Saba, basic skin+VMD          |
| Audio             |   ✔️   | miniaudio                            |
| Material          |   📋   |
| Physics           |   📋   | Planned                              |

| Rendering System                     | ✔️ | 🚧 | 📋 | ❌ | Notes / Progress                                    | Refactor |
|--------------------------------------|:--:|:--:|:--:|:-:|-----------------------------------------------------|:--------:|
| Render Thread Separation             | ✔️ |    |    |   | Decoupled window and rendering threads              |          |
| Dynamic Rendering                    | ✔️ |    |    |   | Framebuffer/Renderpass replaced                     |          |
| Descriptor Pool                      | ✔️ |    |    |   | Global shared set0                                  |          |
| Pipeline Library                     | ✔️ |    |    |   | Pipeline precompile, caching, dynamic switching     |          |
| Timeline Semaphore                   | ✔️ |    |    |   | Timeline semaphore for synchronization              |          |
| Secondary Cmd Parallel (legacy)      |    |    |    | ❌ | Rewritten                                           |          |
| Real-Time RT Shadows                 | ✔️ |    |    |   | RT_KHR + dynamic BLAS                               |          |
| ShadowMap+PCF (legacy)               |    |    |    | ❌ | Orthographic, can't update scene AABB in real time  |          |
| Volumetric Clouds                    | ✔️ |    |    |   | Ray-marching, Shadertoy migrated                    |          |
| Volumetric Fog                       | ✔️ |    |    |   | Raymarching & screen space fog, blue noise sampling |          |
| Deferred Rendering (G-buffer)        | ✔️ |    |    |   | G-buffer structure, deferred shading pipeline       |          |
| Simple Render Graph                  | ✔️ |    |    |   | Basic render graph system implemented               |          |
| Skybox (legacy)                      |    |    |    | ❌ | Cube map, legacy                                    |          |
| Simple Volumetric Fog/Noise (legacy) |    |    |    | ❌ | FastNoiseLite simple implementation                 |          |

| Resource Systems       | ✔️ | 🚧 | 📋 | ❌ | Notes / Progress                                  | Refactor |
|------------------------|:--:|:--:|:--:|:-:|---------------------------------------------------|:--------:|
| LRU Staging Buffer     | ✔️ |    |    |   | LRU upload for better memory/bandwidth usage      |          |
| Async Resource Loading | ✔️ |    |    |   | Multithreaded async loading, avoid main thread    |          |
| Standard Model         | ✔️ |    |    |   | Assimp loading, stored in std::pmr::vector        |          |
| Standard Images        | ✔️ |    |    |   | stb_image loading                                 |          |
| MMD Support            | ✔️ |    |    |   | Implemented via Saba library                      |          |
| Chinese Path Support   | ✔️ |    |    |   | Boost_locale for Windows multilingual path issues |          |

| Toolchain / Editor Systems             | ✔️ | 🚧 | 📋 | ❌ | Notes / Progress                            | Refactor |
|----------------------------------------|:--:|:--:|:--:|:-:|---------------------------------------------|:--------:|
| Shader Hot Reload                      | ✔️ |    |    |   | Monaco-editor + webview, Ctrl+S hot compile |          |
| Shader Hot Reload (legacy)             |    |    |    | ❌ | wx_widget + thread polling for file change  |          |
| 120fps Window Lock (legacy)            |    |    |    | ❌ | Unnecessary, real-time FPS adjustment       |          |
| Window Drag & Docking                  | ✔️ |    |    |   | ImGui docking                               |          |
| Model Basic Manipulation               | ✔️ |    |    |   | ImGuizmo support                            |          |
| Model Selection (ID buffer)            |    |    | 📋 |   | Render ID buffer, mouse mapping via ImGui   |          |
| Model Selection (Ray picking) (legacy) |    |    |    | ❌ | Mouse AABB picking, interactive highlight   |          |

| Vulkan Extension / Feature            | ✔️ | 🚧 | 📋 | ❌ |
|---------------------------------------|:--:|:--:|:--:|:-:|
| VK_LAYER_KHRONOS_validation           | ✔️ |    |    |   |
| VK_EXT_debug_utils                    | ✔️ |    |    |   |
| ShaderInt64 (feature)                 |    |    | 📋 |   |
| SamplerAnisotropy (feature)           |    |    | 📋 |   |
| GeometryShader (feature)              |    |    | 📋 |   |
| RobustBufferAccess (feature)          |    |    | 📋 |   |
| TessellationShader (feature)          |    |    | 📋 |   |
| VK_KHR_swapchain                      | ✔️ |    |    |   |
| VK_KHR_deferred_host_operations       |    |    | 📋 |   |
| VK_KHR_spirv_1_4                      |    |    | 📋 |   |
| VK_KHR_create_renderpass2             |    |    |    | ❌ |
| VK_KHR_pipeline_library               | ✔️ |    |    |   |
| VK_KHR_shader_non_semantic_info       |    |    | 📋 |   |
| VK_EXT_pipeline_creation_feedback     | ✔️ |    |    |   |
| VK_KHR_ray_tracing_pipeline           | ✔️ |    |    |   |
| VK_KHR_acceleration_structure         | ✔️ |    |    |   |
| VK_KHR_buffer_device_address          | ✔️ |    |    |   |
| VK_KHR_synchronization2               | ✔️ |    |    |   |
| VK_KHR_timeline_semaphore             | ✔️ |    |    |   |
| VK_KHR_dynamic_rendering              | ✔️ |    |    |   |
| VK_KHR_pipeline_executable_properties |    |    | 📋 |   |
| VK_EXT_shader_object                  |    |    | 📋 |   |
| VK_EXT_descriptor_indexing            | ✔️ |    |    |   |
| VK_EXT_descriptor_buffer              |    | 🚧 |    |   |
| VK_EXT_transform_feedback             |    |    | 📋 |   |
| VK_EXT_conditional_rendering          |    |    | 📋 |   |
| VK_EXT_graphics_pipeline_library      |    |    | 📋 |   |
| VK_EXT_shader_module_identifier       |    |    | 📋 |   |
| VK_NVX_multiview_per_view_attributes  |    |    | 📋 |   |
| VK_NV_device_generated_commands       |    |    | 📋 |   |
| VK_EXT_mesh_shader                    |    | 🚧 |    |   |
| VK_KHR_dynamic_rendering_local_read   | ✔️ |    |    |   |
| VK_EXT_robustness2                    |    |    | 📋 |   |

| Third-party Libraries  | ✔️ | 🚧 | 📋 | ❌ |
|------------------------|:--:|:--:|:--:|:-:|
| assimp                 | ✔️ |    |    |   |
| boost                  | ✔️ |    |    |   |
| entt                   | ✔️ |    |    |   |
| flecs                  |    |    |    | ❌ |
| glfw                   | ✔️ |    |    |   |
| glm                    | ✔️ |    |    |   |
| mimalloc               | ✔️ |    |    |   |
| miniaudio              | ✔️ |    |    |   |
| nlohmann               | ✔️ |    |    |   |
| oneapi                 | ✔️ |    |    |   |
| ozz                    |    |    | 📋 |   |
| spdlog                 | ✔️ |    |    |   |
| stb                    | ✔️ |    |    |   |
| webview                | ✔️ |    |    |   |
| saba                   | ✔️ |    |    |   |
| imgui-docking/imguizmo | ✔️ |    |    |   |
| cuda                   |    |    | 📋 |   |
| bullet                 |    |    | 📋 |   |
