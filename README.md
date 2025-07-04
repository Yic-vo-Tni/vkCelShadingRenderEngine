# vkCelShadingRenderEngine

## 简述
- 本仓库内容均为个人自学/工程实践的记录，主要为学习和自用，欢迎任何建议和交流反馈！
- 自主学习Vulkan用，欢迎各位指正、指点 (*╹▽╹*)
- **main** 分支：对 **legacy** 分支的重写（没办法组织的太烂了）。因为水平有限，目前只能在 Clang 下运行，MSVC 有 CRT 问题，MinGW 部分库编译不通。
- **legacy** 分支：对 [vkCelShadingRenderer](https://github.com/Yic-vo-Tni/vkCelShadingRenderer) 的重写版，原来那个更烂 = =。

## Description *(Translation by ChatGPT / My English is not good)*
- This repository is a personal record of self-learning and engineering practice, mainly for learning and personal use. Any suggestions or feedback are welcome!
- Mainly used for exploring and learning Vulkan by myself. All comments and advice are appreciated. (╹▽╹)
- **main branch**: A rewritten version of the legacy branch (the previous version was poorly organized). Due to limited experience, it currently only works under Clang; there are CRT issues with MSVC and some libraries fail to compile under MinGW.
- **legacy branch**: A rewritten version of [vkCelShadingRenderer](https://github.com/Yic-vo-Tni/vkCelShadingRenderer). (Unfortunately, that version was even worse = =)

## 学习记录
- 因为对 Vulkan 感兴趣，属于直接硬学Vulkan，C++ 只是在项目推进中按需学习，整体代码水平有限，请多包涵。
- **应用**: 已在项目实际功能中实现并使用
- **了解**: 仅在项目中简单尝试、实践demo
- **计划**: 已有思路未实现，或未来计划学习
- **程度**: 根据对应栏目打勾，简要说明实际掌握或应用的具体深度

- I started learning Vulkan out of personal interest, and focused directly on it; C++ was learned as needed throughout project development. My overall coding proficiency is limited—please forgive any shortcomings.
- **Applied**: Actually implemented and used in project features
- **Familiar**: Only tried or experimented with in simple demos
- **Planned**: Have ideas but not yet implemented, or plan to learn in the future
- **Proficiency**: Marked accordingly, briefly describing the actual level of mastery or usage

> (English translation by ChatGPT. Please refer to the Chinese version if anything is unclear!)

### 00、项目内容
| 事项/内容                       |  实现   |  进行   | 计划 | 备注/进展说明                           |
|--------------------------------|:-----:|:-----:|:----:|:----------------------------------------|
| 事件系统                       |  ✔️   |       |      | 支持事件分发、监听、泛型回调            |
| 全局线程安全存储接口             |  ✔️   |       |      | 多线程下的全局资源/数据访问             |
| shader热重载                    |  ✔️   |       |      | 支持编辑器热编译，GLSL/SPIR-V自动切换   |
| 临时编辑器                      |  ✔️   |       |      | ImGui自研，资源浏览/交互调试            |
| 自由摄像机                      |  ✔️   |       |      | 支持轨道/飞行/第一人称等多模式          |
| 独立渲染线程                    |  ✔️   |       |      | 渲染主循环与逻辑完全解耦                |
| 模型异步加载                    |  ✔️   |       |      | Assimp/Saba多线程加载                   |
| 二级命令单次构建重用             |  ✔️   |       |      | 分帧重用命令缓存                        |
| 单次命令并行提交                 |  ✔️   |       |      | 多队列+TBB并发                          |
| LRU stg buf                     |  ✔️   |       |      | 自研LRU staging buffer池，带宽优化      |
| 实时硬件光线追踪阴影             |  ✔️   |       |      | KHR光追，tlas/blass自动重建             |
| 窗口帧率120fps                  |  ✔️   |       |      | VSync/FPS统计                           |
| 阴影 shadow map, PCF            |  ✔️   |       |      | 前代支持，已集成到主流程                |
| 色调映射                        |  ✔️   |       |      | 支持多种映射曲线                        |
| vmd动画加载播放（Saba库）        |  ✔️   |       |      | 兼容PMX/VMD骨骼/表情/摄像机动画         |
| 模型选取（鼠标射线碰撞检测）     |  ✔️   |       |      | 支持AABB拾取、交互高亮                  |
| 天空盒                          |  ✔️   |       |      | 支持立方体贴图、昼夜切换                |
| 简单体积雾 柏林噪声              |  ✔️   |       |      | fastNoiseLite库，支持实时体积特效        |
---

### 01、C++
| 语法/特性相关                   | 应用 | 了解 | 计划 | 程度/说明                              |
|--------------------------------|:----:|:----:|:----:|:---------------------------------------|
| C++98/03 语法                    |      | ✔️   |      | 了解历史写法，兼容老库                 |
| C++11 语法/特性                    | ✔️   |      |      | auto, lambda, range-for, 智能指针用法  |
| C++14/17 语法/特性                 | ✔️   |      |      | 结构化绑定，std::optional, variant等   |
| C++20/23 语法/特性                 |      | ✔️   |      | concept, ranges, coroutine 用过demo    |
| STL 所有容器                       | ✔️   |      |      | vector, map, set, deque, priority_queue等 |
| STL 算法（sort/find/transform等）   | ✔️   |      |      |                                        |
| 智能指针 unique_ptr/shared_ptr     | ✔️   |      |      | 资源生命周期管理                       |
| Lambda/闭包/回调/可捕获变量             | ✔️   |      |      | 事件系统、UI回调                       |
| operator重载                     | ✔️   |      |      | 迭代器/数据容器/数值类型等             |
| 类/继承/多态/虚函数                    | ✔️   |      |      | 各类系统/工厂/接口抽象                 |
| 函数对象/functor                   | ✔️   |      |      | operator()重载                         |
| 模板函数/类                         | ✔️   |      |      | traits、事件泛型封装                   |
| 函数模板特化/偏特化                     |      | ✔️   |      | function_traits自动识别回调参数类型     |
| SFINAE/enable_if/decltype/auto |      | ✔️   |      | updateCondition等                      |
| constexpr/consteval/constinit  |      | ✔️   |      | 静态常量、编译期表达式                 |
| static_assert/type_traits      | ✔️   |      |      | 类型萃取、编译期检查                   |
| C++ RTTI/typeid/dynamic_cast   | ✔️   |      |      | 组件类型判定、事件分发                  |
---
| 并发与内存/工程化              | 应用 | 了解 | 计划 | 程度/说明                        |
|------------------------------|:----:|:----:|:----:|:---------------------------------|
| 自定义allocator/内存池        |      |      | ✔️   | 了解/看过源码，未集成工程         |
| move语义/右值引用             | ✔️   |      |      | 资源高效转移                     |
| 线程/thread/mutex/lock        |      | ✔️   |      | 用过std::thread/lock，主用TBB     |
| atomic/volatile/内存序        |      | ✔️   |      | TBB并发数据结构/atomic flag       |
| condition_variable/semaphore  |      |      | ✔️   | 看过线程同步示例                  |
| TBB/oneTBB/并行算法           |      | ✔️   |      | 任务调度，资源并发上传            |
| future/promise/async          |      |      | ✔️   | 了解原理，未主流程集成            |
| std::function/std::bind       | ✔️   |      |      | UI和ECS系统回调                   |
| 异常/try-catch/自定义异常     | ✔️   |      |      | 资源/逻辑异常处理                 |
| assert/静态断言              | ✔️   |      |      | 参数校验、单元测试                |
---
| 模板/类型/元编程/反射/设计模式 | 应用 | 了解 | 计划 | 程度/说明                        |
|------------------------------|:----:|:----:|:----:|:---------------------------------|
| 类型萃取/traits设计           |      | ✔️   |      | function_traits/成员遍历          |
| CRTP/curiously recurring template |      |      | ✔️   | 只在看源码时分析过                |
| 反射/自动序列化               |      | ✔️   |      | boost::hana成员反射               |
| 设计模式（单例/工厂/观察者/策略等） | ✔️   |      |      | 单例（系统/资源/渲染库），工厂/回调用 |
---
| 工具链/工程自动化             | 应用 | 了解 | 计划 | 程度/说明                           |
|------------------------------|:----:|:----:|:----:|:------------------------------------|
| CMake/编译脚本                |      | ✔️   |      | shader自动编译/自动测试              |
| 代码生成/元编程                |      |      | ✔️   | 了解宏生成/代码生成工具，未在主工程用 |
| 代码风格/静态分析/linter       |      | ✔️   |      | Clang-format, 静态检查工具           |
---
### 02、Vulkan/图形API

| Vulkan 基础构建                | 应用 | 了解 | 计划 | 程度/说明              |
| -------------------------------- | :--: | :--: | :--: | -------------------- |
| Instance/Device/Surface         | ✔️   |      |      | 启动流程、窗口创建、自封装 |
| vk-hpp语法/自动析构               | ✔️   |      |      | 资源全RAII             |
| Physical/LogicalDevice管理      | ✔️   |      |      | 多GPU选择、功能查询       |
| Queue/CommandPool/CommandBuffer | ✔️   |      |      | 多线程绘制、命令复用       |
| Swapchain/Framebuffer           | ✔️   |      |      | resize自动重建、失效恢复   |
| RenderPass/Subpass/Attachment   | ✔️   |      |      | 多渲染目标、前后处理       |
| Pipeline/PipelineLayout/Shader  | ✔️   |      |      | 多管线灵活切换           |
| DescriptorSet/Pool/Layouts      | ✔️   |      |      | 多池分配、动态资源绑定     |
| PushConstant/UniformBuffer      | ✔️   |      |      | 小数据推送、矩阵/参数传递   |

---

| Vulkan 内存/资源/同步                 | 应用 | 了解 | 计划 | 程度/说明                 |
|---------------------------| :--: | :--: | :--: | --------------------- |
| Buffer/Image管理           | ✔️   |      |      | 统一allocator、生命周期管理  |
| BufferView/Map/Unmap      | ✔️   |      |      | 动态数据同步/异步上传      |
| ImageView/布局转换         | ✔️   |      |      | 各阶段自动Layout切换       |
| 多帧同步/Semaphores/Fences | ✔️   |      |      | 多帧管线自动同步           |
| AccelerationStructure管理 | ✔️   |      |      | 光追场景结构、动态创建/释放 |
| MultiQueue/多队列         |      | ✔️   |      | 理论了解，未主流程应用      |
| 动态描述符(bindless)      | ✔️   |      |      | 多资源动态上传/绑定        |
| MSAA/多采样抗锯齿         | ✔️   |      |      | shader参数配置、后处理     |
| Sampler/纹理采样           | ✔️   |      |      | 动态绑定、材质系统         |

---

| Vulkan 扩展/高级特性                  | 应用 | 了解 | 计划 | 程度/说明                   |
|-------------------------------| :--: | :--: | :--: | ----------------------- |
| 着色器模块/热重载                | ✔️   |      |      | ShaderEditor、热更新     |
| RayTracing（KHR）              | ✔️   |      |      | 光追Pipeline、阴影         |
| VK_KHR_dynamic_rendering       | ✔️   |      |      | RenderPass2CI适配        |
| timeline semaphore            |      |      | ✔️   | 计划并行提交、信号量替换      |
| MeshShader/DescriptorBuffer等扩展 |      | ✔️   |      | 文档/源码，未主流程集成    |
| Shader编译/调试/反编译            | ✔️   |      |      | spirv编译/反编译/调试工具  |
| ValidationLayer/调试扩展         | ✔️   |      |      | VUID定位、性能警告        |
| 性能分析/Profile/调优            |      | ✔️   |      | GPU Profile分析，待深入   |
| Vulkan多平台API抽象              |      | ✔️   |      | DX12/Metal原理分析        |
| DeviceGroup/多GPU               |      | ✔️   |      | 概念了解，未工程应用       |

---

| Vulkan 优化/经验/对比              | 应用 | 了解 | 计划 | 程度/说明                 |
|--------------------------| :--: | :--: | :--: | --------------------- |
| 内存对齐/性能/映射         | ✔️   |      |      | Buffer分配/对齐优化      |
| Shader优化（分支/unroll等）| ✔️   |      |      | 着色器代码调优           |
| Vulkan与OpenGL/DirectX对比 |      | ✔️   |      | API接口/原理差异         |
---

### 03、渲染/引擎系统/图形学
| 渲染系统与场景                  | 应用 | 了解 | 计划 | 程度/说明                        |
|--------------------------------|:----:|:----:|:----:|:-------------------------------|
| 相机系统/投影矩阵/正交透视       | ✔️   |      |      | 支持交互旋转/缩放               |
| 视锥剔除/包围盒AABB/OBB          |      | ✔️   |      | 动态场景管理/裁剪               |
| 网格管理/分批绘制                | ✔️   |      |      | 顶点/索引/多submesh支持         |
| 渲染队列/排序                    | ✔️   |      |      | DrawCall按类型/优先级排序       |
| 跨平台资源格式支持               |      | ✔️   |      | PMX/VMD/Assimp                  |
| 项目结构/模块化/插件化           | ✔️   |      |      | 代码分层，单例工厂               |
| 跨平台/宏控制                    |      | ✔️   |      | 兼容Win/Linux                   |
| 文件系统/虚拟文件系统             |      | ✔️   |      | VFS了解                         |
---
| 材质/动画/特效/后处理              | 应用 | 了解 | 计划 | 程度/说明                       |
|----------------------------------|:----:|:----:|:----:|:-------------------------------|
| 材质系统/参数绑定                | ✔️   |      |      | Descriptor+ImGui切换            |
| 动画系统/骨骼蒙皮/插值            |      | ✔️   |      | PMX/VMD骨骼动画，关键帧插值      |
| 骨骼层级/蒙皮矩阵缓存             |      | ✔️   |      | 层级递归，减少重复计算           |
| 粒子系统/实例化/批量绘制          |      | ✔️   |      | demo学习/未实装                 |
| 光照模型（卡通/Blinn/Phong/PBR）  |      | ✔️   |      | 卡通shader自写，PBR了解         |
| 阴影（ShadowMap/光追阴影）        |      | ✔️   |      | RT阴影，基本投影实现            |
| 屏幕空间特效（SSAO/Bloom/TAA）    |      | ✔️   |      | 算法文档学习，未写demo           |
| 体积/大气/水面                    |      | ✔️   |      | VolumetricClouds shader         |
| 后处理/色调映射                   |      | ✔️   |      | Post通道，ImGui切换特效         |
| LOD/动态细分/剔除                 |      | ✔️   |      | 资料/源码分析，未工程用          |
---
| 编辑器/交互/引擎功能              | 应用 | 了解 | 计划 | 程度/说明                       |
|----------------------------------|:----:|:----:|:----:|:-------------------------------|
| 实时编辑器/操作器                | ✔️   |      |      | ImGui/ImGuizmo交互编辑          |
| UI框架/自定义控件                | ✔️   |      |      | ImGui全流程封装                 |
| ECS核心/多线程遍历               | ✔️   |      |      | entt全流程，tbb并行支持         |
| 资源热重载/热更新                |      | ✔️   |      | shader/配置/动画热更            |
| 动态资源加载/异步流              |      | ✔️   |      | tbb并发，解耦主线程             |
| 渲染统计/性能测量                |      | ✔️   |      | 帧率/FPS/内存/带宽等统计         |
| 自动化测试/单元测试              |      | ✔️   |      | 学习过gtest用法                 |
| 网络通信/同步                    |      |      | ✔️   | socket/http/未项目用            |
---
### 04、工具链/工程/平台适配/生态极细分

| 工程管理与构建                  | 应用 | 了解 | 计划 | 程度/说明                      |
|--------------------------------|:----:|:----:|:----:|:-----------------------------|
| CMake/构建脚本                 |      | ✔️   |      | shader自动编译                 |
| Git/版本管理                   | ✔️   |      |      | 全项目用Git管理                |
| linter/静态分析/格式化          |      | ✔️   |      | clang-format                  |
| 日志系统/错误追踪               | ✔️   |      |      | yic::logger集成               |
| 文档/注释/开发手册              | ✔️   |      |      | 工程内有注释/自动文档          |
---
| 发布/兼容/扩展/集成               | 应用 | 了解 | 计划 | 程度/说明                      |
|----------------------------------|:----:|:----:|:----:|:-----------------------------|
| 跨平台打包/发布                  |      | ✔️   |      | 了解打包，未正式发布           |
| 插件/脚本/扩展系统                |      | ✔️   |      | 关注UE/Unity实现              |
| 第三方库集成(Assimp/ImGui/GLFW)  | ✔️   |      |      | assimp, imgui, glfw深度集成   |
| 数据格式转换/导出                 |      | ✔️   |      | 纹理/模型格式                  |
| 性能工具/profile/benchmark        |      | ✔️   |      | API profile/帧率统计           |
| 内存检测/泄漏检测                 |      | ✔️   |      | valgrind理论                   |
| 兼容性测试/平台适配               |      | ✔️   |      | DX12/Metal文档分析             |
| 脚本语言绑定(Python/Lua)          |      | ✔️   |      | pybind11/lua绑定概念           |
| CI/CD自动部署                     |      |      | ✔️   | 概念了解，未工程用             |
---
### 05、动画/物理/AI/其他
| 动画/物理/AI/场景/扩展         | 应用 | 了解 | 计划 | 程度/说明                        |
|------------------------------|:----:|:----:|:----:|:-------------------------------|
| 动画/BlendTree/状态机         |      | ✔️   |      | 多动画切换，简单状态             |
| AI/寻路/行为树                |      | ✔️   |      | demo用过A*                      |
| 物理/软体/布娃娃              |      |      | ✔️   | 理论了解                        |
| 粒子物理/布料模拟             |      | ✔️   |      | 资料学习                        |
| 道具系统/事件驱动AI           |      | ✔️   |      | UE源码分析                      |
| 场景管理/分区加载             |      | ✔️   |      | 动态加载/资源调度                |
| 编辑器扩展/脚本API            |      | ✔️   |      | UE/Unity学习经验                |
| 工具链/批处理/导出            |      | ✔️   |      | 自定义文件导入导出               |
---



  




