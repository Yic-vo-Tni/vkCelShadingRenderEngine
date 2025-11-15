# Hakuro Fabric

🔎 对于非关键性错误、轻微优化与 TODO，请参见  
[已知问题与计划任务 (Known Issues & TODOs)](KnownIssues.md)

## 概览（Overview）

- 本仓库是作者自学与工程实践的个人记录，主要用于学习与个人研究使用。 欢迎任何建议与反馈！
- 项目主要用于独立探索与学习 Vulkan，所有的评论与意见都非常欢迎！(╹▽╹)

### 截图（Screenshots）

<img src="screenShot/01.png" width="340" alt=""/> <img src="screenShot/02.png" width="340" alt=""/>

---


### 学习目标与功能规划（2025.9.6）

> *说明：以下为学习目标或计划，仍在持续学习中，尚未在项目中完整实现（或者进展还很遥远(⊙︿⊙)）。*

**Hakuro Fabric = Architecture * Atmosphere * Elegance + Bonus**

---

- **System Graph（系统图）**

  用于统一管理与调度全局系统，包括渲染图等子系统。

> 当前理解：渲染图可以自动分配并派发任务到工作线程系统，所有的构建工作在渲染线程外提交，  
> 进一步解耦渲染线程，目标是实现零阻塞（Zero-Blocking）。

---

- **Advanced Vulkan Extensions（高级 Vulkan 扩展）**

  GPU 侧的并行命令缓冲构建机制。

> 当前理解：新的 Vulkan 工作流包括: dll + dy_dispatch + swapchain maintain + pipeline library +
> dynamic rendering + descriptor buffer + GPU command generation + timeline semaphores + VMA 3-pool + mesh shader +
> shader obj.

---

- **High-End Rendering Effects（高级渲染特效）**

  体素网格体积雾 / 云，以及基于物理的预计算大气模型。

> 当前理解：逐步放弃 Shadertoy 的 Hack 方案，转而学习 IQ 的渲染教程。  
> 学习路径为：小立方体体积 → 切片 (slab) → 世界空间噪声 → 光照 → 体素化。

---

- **Animation System（动画系统）**

  拆解 Saba 库，用于后续在 VR、物理、布料等方向的实验性研究。

> 当前理解：首先仅提取动画数据结构以整合进 ECS 系统，  
> 其他部分暂时保持不动。第一步是将数据层与原库进行解耦。

---

- **Optimization & AI**
> too far
  

---

### 当前项目状态（2025.11.11）

> 每个条目使用以下标识之一：
>
> - ✔️  已实现：具备基础功能并可正常使用（但可能未完全完善）；
> - 📋  进行中：已部分实现或正在规划中；
> - ❌  已废弃：功能暂时性失效或者移除，不再维护。
>
> 本章节反映了引擎的 **实际进度与核心特性**，  
> 而非长期目标或实验性原型。


| Core Systems | Status | Notes / Progress                                       | Refactor / Plan              |
|--------------|:------:|--------------------------------------------------------|------------------------------|
| Event        |   ✔️   | 全局事件系统（发布/订阅），异步分发，唯一事件类型。支持多线程安全延迟事件调度，用于帧级同步与事件堆积处理。 |                              |
| Task         |   📋   |                                                        | 计划基于 TBB Graph 构建全局任务调度系统    |
| Storage      |   ✔️   | 线程安全存储模块，支持类型安全指针访问与多线程锁保护。                            |                              |
| ECS          |   ✔️   | 基于 EnTT 的模块化实体组件系统，具备组件注册与快速查询能力。                      |                              |
| Threads      |   ✔️   | 拥有窗口、逻辑（快/慢）与渲染线程，采用三缓冲环形索引进行同步。                       | 计划拆分渲染线程逻辑，解耦命令录制与提交阶段       |
| Animation    |   ✔️   | 基于 Assimp + Saba，支持 GPU 蒙皮动画与 VMD 动画(CPU蒙皮），多模型并行播放。   | 拆解 Saba，统一动画数据至 ECS 体系       |
| Audio/video  |   ✔️   | 使用 miniaudio 实现音频播放与暂停。                                | 计划支持 mp4 导出并整合 ffmpeg 进行视频输出 |
| Material     |   📋   |                                                        | 创建文件夹中ing                    |
| Physical     |   📋   | 尚未实现，处于预研阶段。                                           | 计划学习并集成 Jolt 物理引擎            |
| Scene        |   ✔️   | 场景管理系统支持全局 TLAS 构建；模型增删时可动态更新 TLAS。                    | 拆分动态 TLAS 与静态 TLAS           |
| Light        |   📋   | 已构建基础光照框架，使用球体模拟光源。                                    | 完善光源（先实现点光源）                 |

| Rendering System   | Status | Notes / Progress                                       | Refactor / Plan                          |
|--------------------|:------:|--------------------------------------------------------|------------------------------------------|
| Dynamic Rendering  |   ✔️   | 基于 VK_KHR_dynamic_rendering，无需 Framebuffer，完全使用动态渲染管线。 |                                          |
| Render Graph       |   ✔️   | dsl搭建，支持自动依赖推断与拓扑排序。                                   | 引入每pass性能分析；支持动态可编辑                      |
| Deferred Rendering |   ✔️   | 标准 G-Buffer 管线                                         |                                          |
| Sync (Semaphore)   |   ✔️   | 使用时间线信号量与 VK_KHR_synchronization2 实现全帧同步。              | 后续做多队列时候可能需要优化一点                         |
| Descriptor System  |   ✔️   | 全局 Set0 管理 + Bindless 资源绑定                             | 迁移至 VK_EXT_descriptor_buffer             |
| Pipeline Library   |   ✔️   | 管线库系统，支持预编译与动态切换。                                      | 计划 用Shader Object 优化热重载                  |
| Allocator          |   ✔️   | 基于 VMA 实现，支持 LRU 回收与内存池管理。                             | 计划拆分为三层内存池                               |
| Command System     |   ✔️   | 命令系统第二版：线程安全并支持和渲染图并行构建，针对单次构建也最终统一提交                  | 支持多队列并行以及二级命令                            |
| FIFO-latest        |   ❌    | 在高帧率下出现 Swapchain 同步问题。                                | 暂不修复                                     |
| RT Shadows         |   ✔️   | 使用 VK_KHR_ray_tracing，实现动态 BLAS 定向光阴影。                 | 支持多光源阴影                                  |
| Shadow Map         |   ❌    | 光栅阴影绘制实现未完成，暂时搁置（已由 RT 阴影替代）。                          | 若后续需要可恢复开发                               |
| Volumetric Clouds  |   ✔️   | 基于 Ray-Marching，Shadertoy 效果移植。                        | 计划学习并整合预计算大气散射模型（Precomputed Atmosphere） |
| Volumetric Fog     |   ✔️   | 屏幕空间体积雾和噪音体积雾效果。                                       | 准备升级为半体素（Semi-Voxel Grid）体积雾系统           |
| HDR                |   ❌    | HDR 帧缓冲格式与后处理图像已替换，但视觉效果似乎没起效果。                        | 暂时搁置                                     |
| Mesh Shader        |   📋   | 准备替换传统顶点着色器管线。                                         | 引入 Meshlet 与 Task Shader 结构              |

| Rendering System   | Status | Notes / Progress                                       | Refactor / Plan                       |
|--------------------|:------:|--------------------------------------------------------|---------------------------------------|
| Dynamic Rendering  |   ✔️   | 基于 VK_KHR_dynamic_rendering，无需 Framebuffer，完全采用动态渲染管线。 |                                       |
| Render Graph       |   ✔️   | DSL 架构搭建，支持自动依赖推断与拓扑排序。                                | 引入每 Pass 性能分析；支持动态可编辑                 |
| Deferred Rendering |   ✔️   | 标准 G-Buffer 管线。                                        |                                       |
| Sync (Semaphore)   |   ✔️   | 使用时间线信号量与 VK_KHR_synchronization2 实现全帧同步。              | 多队列结构下可能需要进一步优化                       |
| Descriptor System  |   ✔️   | 全局 Set0 管理 + Bindless 资源绑定。                            | 迁移至 VK_EXT_descriptor_buffer          |
| Pipeline Library   |   ✔️   | 管线库系统，支持预编译与热重载。                                       | 计划使用 Shader Object 优化热重载              |
| Allocator          |   ✔️   | 基于 VMA 实现，支持 LRU 回收与内存池管理。                             | 拆分为三层内存池                              |
| Command System     |   ✔️   | 命令系统第二版：线程安全，支持与渲染图并行构建；单次构建也最终统一提交。                   | 支持多队列并行与二级命令缓冲                        |
| FIFO-latest        |   ❌    | 在高帧率下出现 Swapchain 同步问题。                                | 暂不修复                                  |
| RT Shadows         |   ✔️   | 使用 VK_KHR_ray_tracing，实现动态 BLAS 定向光阴影。                 | 支持多光源阴影                               |
| Shadow Map         |   ❌    | 光栅阴影绘制尚未完成，暂时搁置（已由 RT 阴影替代）。                           | 若后续需要可恢复开发                            |
| Volumetric Clouds  |   ✔️   | 基于 Ray-Marching，Shadertoy 效果移植。                        | 计划整合预计算大气散射模型（Precomputed Atmosphere） |
| Volumetric Fog     |   ✔️   | 屏幕空间体积雾与噪声体积雾效果。                                       | 准备升级为半体素（Semi-Voxel Grid）体积雾系统        |
| HDR                |   ❌    | HDR 帧缓冲与后处理图像已替换，但视觉效果尚未生效。                            | 暂时搁置                                  |
| Mesh Shader        |   📋   | 准备替换传统顶点着色器管线。                                         | 引入 Meshlet 与 Task Shader 结构           |

| Resource Systems       | Status | Notes / Progress                                 | Refactor / Plan                      |
|------------------------|:------:|--------------------------------------------------|--------------------------------------|
| Async Resource Loading |   ✔️   | 异步资源加载系统，避免阻塞主线程，支持屏障同步。                         | 并行化加载流程；统一构建 TLAS 资源管理逻辑             |
| Standard Model         |   ✔️   | 使用 Assimp 进行模型导入，采用 std::pmr::vector 管理内存。       | 计划结合 mimalloc 与 pmr 实现高效模型内存分配       |
| Standard Images        |   ✔️   | 基于 stb_image 的图像加载模块。                            | 拆分 Image 类，将部分功能迁移至实时渲染（RT）层         |
| MMD Support            |   ✔️   | 基于 Saba 的 MMD 实现，支持 PMX/VMD 动画加载。                | 与 Assimp 模型系统采用相同的重构计划               |
| Chinese Path           |   ✔️   | 使用 Boost.Locale 解决 Windows 中文路径兼容问题。             | 移除 locale 依赖（过度工程化方案）                |
| Resource Pool          |   📋   | 使用 std::pmr::unsynchronized_pool_resource 管理内存池。 | 拆分为多池结构以减少碎片化，整合 std::pmr 与 mimalloc |

| Toolchain / Editor Systems  | Status | Notes / Progress                           | Refactor / Plan                 |
|-----------------------------|:------:|--------------------------------------------|---------------------------------|
| Shader Hot Reload           |   ✔️   | 集成 Monaco 编辑器 + WebView，支持 Ctrl+S 实时编译着色器。 | 迁移至 VK_EXT_shader_object 实现即时替换 |
| Window Drag & Docking       |   ✔️   | 基于 ImGui 的 Docking 系统，支持窗口拖拽与布局保存。         |                                 |
| Model Basic Manipulation    |   ✔️   | 集成 ImGuizmo，支持模型的平移、旋转与缩放操作。               |                                 |
| Model Selection (ID buffer) |   ✔️   | 通过渲染 ID 缓冲实现模型选取，ImGui 鼠标映射至 EnTT 实体。      | 支持多选与高级选取逻辑                     |
| Debug Output                |   📋   | 计划添加 Vulkan 调试输出与 GPU 标记系统。                |                                 |
| Folder                      |   📋   |                                            | 用于在编辑器中展示引擎内部资源结构               |
| NodeGraph                   |   ✔️   | 可视化渲染流程与各 RT 依赖关系展示。                       | 支持运行时动态编辑与修改                    |

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
| ffmpeg                 |    |    | 📋 |   |                  |

### 旧版 / 已废弃功能（Legacy / Deprecated Features）

> 以下功能在之前渲染器实现，当做学习的记录。 

| 旧版功能（Legacy Feature）                          | 移除原因或说明                         |
|-----------------------------------------------|---------------------------------|
| Secondary Cmd Parallel（次级命令缓冲并行）              | 已重写                             |
| ShadowMap + PCF（阴影贴图 + PCF）                   | 被实时光线追踪阴影取代                     |
| Skybox（天空盒）                                   | 已废弃，替换为体积云                      |
| Simple Volumetric Fog/Noise（简易体积雾 / 噪声）       | 使用 FastNoiseLite 实现，已被完整体积雾系统取代 |
| wx_widget Shader Reload（基于 wxWidgets 的着色器热重载） | 被 Monaco Editor + WebView 方案取代  |
| 120fps Window Lock（窗口帧率锁定）                    | 已移除，新线程系统下不再需要                  |
| Model Selection (Ray Pick)（射线拾取模型）            | 被基于 ID Buffer 的选取机制替代           |

---

### 开发理念（Development Philosophy）

本项目通过持续的迭代与重构循环不断演进， 许多系统在不同阶段（水平）被实现，因此部分模块在代码风格或抽象层级上可能存在不一致。
有些存在多个版本，比如说set和cmd系统有两个版本，有些前面写的太烂了，不方便一次性删干净整理完逻辑，所以写了第二版等等，后续
会逐渐整理优化好的，现在也有很多TODO，FIXME，HACK等，标注起来方便自己查看，有的只是因为能力问题临时这么实现，可能比较烂，请多多包含。：）

---

## 构建方式（How to Build）

本项目主要用于个人学习，因此始终面向 **最新工具链与硬件环境**。  
请确保开发环境 **不低于作者当前环境版本**，否则可能无法正常编译。

### 开发环境（2025.11.9）
- Vulkan SDK：1.4.312　*(最低需 1.3 或更高版本)*
- Vulkan Runtime：1.4.312
- CMake：3.29　*(3.20 及以上版本可用)*
- 编译器：Clang 21.1.1  
  *(Clang 18+ 可用；由于 TBB 限制，不支持 MinGW；MSVC 可能遇到 CRT 兼容性问题)*
- GPU：NVIDIA GeForce RTX 3080 Ti *(RTX 20 系列理论可行，AMD 暂未测试)*
- 操作系统：Windows 11

---

### 外部依赖（2025.9.4）

当前需手动下载或构建以下依赖库：  
`Assimp, Boost (locale/system), mimalloc, entt, glfw, oneAPI TBB, glm, miniaudio, nlohmann, spdlog, stb, webview, imgui(docking/nodes), imguizmo, saba, bullet`。

- 将源码放入 `third/xxx` 文件夹中；
- 已构建的 `.lib` 放入 `third/lib`，`.dll` 放入 `third/dll`。

📘 说明：
- **Bullet**：需遵循 Saba 的编译依赖要求。
- **Saba**：需清理内部自带的 spdlog，以避免与外部冲突。
- 所有依赖库配置完成后，可直接在 CLion 中打开并构建项目。
- 后续计划：移除 Boost 依赖，改用 **git submodules** 管理。

> 由于作者的学习重点是 Vulkan，许多功能会优先通过引入库来实现。  
> 即便某些库仅使用其中一小部分功能，因为作者水平问题，有些有点小题大做了。
> 构建太繁琐了，会计划改成一站式。

---

### 验证报告（Validation Report, 2025.9.4）

- **Errors：** 无
- **Warnings：** 无 *(除了最小化窗口)*
- **Best Practices：** 无

📌 说明：  
引擎中与 Vulkan 相关的部分均经过严格验证，保证干净无误。  
其他子系统（如动画、编辑器逻辑、资源加载）可能仍存在错误或未完成的逻辑。  
这些模块属于次要部分，有更好的想法或者无聊的时候再修。
