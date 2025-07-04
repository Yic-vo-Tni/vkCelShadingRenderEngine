# vkCelShadingRenderEngine

## 简述
- 自学用，欢迎指导指导我
- main分支重写的但只能Clang运行，msvc有CRT问题，MinGW第三方库tbb编译有问题
- legacy分支主msvc，MinGW应该也能运行，写的太烂放弃了 

## 内容

### 已做
- 事件系统
- 全局线程安全存储接口
- shader热重载
- 临时编辑器
- 自由摄像机
- 独立渲染线程
- 模型异步加载
- 二级命令单次构建重用
- 单次命令并行提交
- LRU stg buf
- 实时硬件光线追踪阴影
- 窗口帧率120fps

#### 项目前身做过
- 阴影 shadow map, PCF
- 色调映射
- vmd动画加载播放 库：Saba
- 模型选取（鼠标射线碰撞检测场景AABB）
- 天空盒
- 简单体积雾 柏林噪声 库：fastNoiseLite

### 正在做
#### 首要
- id buffer 选取模型
- rt 静态场景单次绘制
- 重写队列类，配合time line sem并行提交
- 重写descriptor类

#### 其次
- 基础PBR
- MSAA

#### 逐步前进
- 大气渲染 参考 https://ebruneton.github.io/precomputed_atmospheric_scattering/
  - 目标：复刻Matěj Sakmary 的 Atmosphere and Cloud Rendering in Real-time

#### 逐步重构
- 逐步替换stl内存分配器为mimalloc
- 逐步替换二进制信号量为 timeline semaphore 扩展
- 逐步替换使用 dynamic render pass 扩展
- 逐步替换使用 descriptor index 扩展

### 计划：想做
- 高级PBR：SSS，IBL等
- 延迟渲染
- 项目序列化 json
- ozz动画
- 用Bullet
- 添加音频

#### 额外计划：
- Occlusion Culling
- 云层渲染，雨滴效果

## 构建 
- 我用的是win11，mingw 12.0, cmake 3.28
- 尝试过学习接触avx指令集，需要处理器支持 :)
- 不自动，FetchContext拉取的首次cmake后编译完还需要再cmake一下自动拉取dll文件 = - =

## 使用的库
- vulkan sdk
- glfw
- assimp
- boost
- flecs
- glm
- magic_enum
- mimalloc
- nfd
- oneapi::tbb
- spdlog
- stb
- vmd
- wxWidget
- sfml
- imgui

## 使用的vulkan扩展
- vk_khr_swapchain
- vk_khr_deffered_hostoperations
- vk_khr_acceleration_structure
- vk_khr_buffer_device_address
- vk_khr_ray_tracing_pipeline
- vk_khr_synchronization_2
- vk_khr_spirv_1_4
- vk_khr_timeline_semaphore
### 计划用
- vk_khr_create_renderpass_2
- vk_nv_framebuffer_mixed_samples
- vk_khr_dynamic_rendering
- vk_khr_descriptor_index

## C++ 23


  




