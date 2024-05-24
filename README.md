# vkCelShadingRenderEngine

## 简介

之前那个渲染器写的太烂，受不了了，重写

这个项目可以直接clone，然后在CLion里面运行。需要C++23（虽然我不确定是否用到了C++23的特性，但用了C++20，所以至少需要C++20）。
用了很多lambda表达式，可能不熟悉lambda的阅读会不太舒服。

## 记录

### 2024.5.23

- **多线程事件系统**：目前看起来挺不错的，能支持多发布多订阅。而且调整窗口时能后台独立输出信息。
  - 通过事件总线的方式做的。
- **基础输入测试**：搞了个键盘输入测试，现在按键可以只响应一次了，还可以多次撤回。


- **添加了Return Type**：这下可以创建了直接返回了，舒服多了，有问题，修改成auto这些方便多了
```c++
template<typename Func>
    auto operator=(Func &&func) {
        try {
            auto r = func();

            if_debug {
                switch (mLevel) {
                    case spdlog::level::info:
                        vkInfo("{0}, successfully", mDescription);
                        break;
                    case spdlog::level::warn:
                        vkWarn("{0} successfully", mDescription);
                        break;
                    case spdlog::level::err:
                        vkError("{0} successfully", mDescription);
                        break;
                    default:
                        vkTrance("{0} successfully", mDescription);
                }
            };

            return r;
        } catch (const vk::SystemError &e) {
            if_debug vkError("failed to {0}: {1}", mDescription, e.what());
            exit(EXIT_FAILURE);
        }
    }
```
- vkCreate返回创建的UniqueInstance,再也不用用一个临时的vk::instance了，之前忘了给try-catch搞个返回值= =
```c++
mInstance([&, appInfo = [&]() {
                return vk::ApplicationInfo{
                        "Yic", VK_MAKE_VERSION(1, 0, 0),
                        "Vot", VK_MAKE_VERSION(1, 0, 0),
                        VK_MAKE_API_VERSION(0, 1, 3, 0)
                };
            }()]() {
                createInfo->addInstanceExtensions(fn::addRequiredExtensions());
                fn::checkInstanceSupport(createInfo->mInstanceExtensions, createInfo->mInstanceLayers);

                return vkCreate("create instance") = [&]() {
                    return vk::createInstanceUnique(
                            vk::InstanceCreateInfo()
                                    .setPApplicationInfo(&appInfo)
                                    .setPEnabledExtensionNames(createInfo->mInstanceExtensions)
                                    .setPEnabledLayerNames(createInfo->mInstanceLayers)
                    );
                };
            }())
```
- 渲染和主窗口各一个线程，这下应该不用像上一个点击窗口会暂停画面了，舒服了
```c++
     [&, rhi_thread = [&]() {
            return std::make_unique<std::thread>([&] {
                try {
                    mRhi->run();
                } catch (const vk::SystemError &e) {
                    std::cerr << e.what() << "\n";
                }
            });
        }(), window_main_thread = [&]() {
            return !vkWindow::run();
        }()
        ]() {
            if (!window_main_thread)
                mRhi->setRunCondition();

            if (rhi_thread->joinable())
                rhi_thread->join();
        }();
```





