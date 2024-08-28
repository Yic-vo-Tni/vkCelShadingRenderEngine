# vkCelShadingRenderEngine

## 内容

- **多线程事件总线**：
    - 全局变量存储更新提取
    - 多订阅多发布
    - 延迟订阅、延迟首次执行
- **多线程任务总线**：
    - 可以按枚举值顺序执行
    - 单个枚举值下多个任务可选并行
    - 支持任务嵌套
- **事件处理**：
    - 可执行撤回操作
- **shader热重载**：
    - 修改shader文件，自动编译替换，重建管线
    - 临时编辑器
- **模型加载**：
    - 并行无锁加载，并行数8
    - 重用stgbuf,采用LRU
- **渲染管理**：
    - 实时窗口重建
    - 渲染独立线程
- **性能优化**：
    - 窗口帧率120fps

## 构建

- 需要Vulkan SDK
- 需要MinGW，CMake、Git
- 需要自行下载vma，tbb（版本忘了）= =，imgui v1.90.9-docking, wx v3.2.5，boost的hana、filesystem、preprocessor,
    - filesystem、tbb、wx需要自行编译，tbb用的是oneapi文件夹的版本
    - 开始的时候没打算用CMake的FetchContext功能，之前给我留下了不好映像 ：）
- 其他库会通过CMake的FetchContext功能自动拉取
    - 编译完自动下载dll
  




