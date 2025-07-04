//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_VKINIT_H
#define VKCELSHADINGRENDERER_VKINIT_H

namespace rhi {

    class VkInit {
    public:
        struct CreateInfo{
        public:
            auto addInstanceExtensions(const auto& extensions){
                addExOrLay(extensions, instanceExtensions);
                return *this;
            }

            auto addInstanceLayers(const auto& layers){
                addExOrLay(layers, instanceLayers);
                return *this;
            }

            template<typename Feature = std::nullopt_t>
            auto addPhysicalExtensions(const auto& extensions, Feature* feature = nullptr){
                addExOrLay(extensions, physicalExtensions);
                if constexpr (!std::is_same_v<Feature, std::nullopt_t>)
                    addFeatureToChain(feature);
                return *this;
            }

            auto addPhysicalFeatures(vk::PhysicalDeviceFeatures2 f){
                features2 = f;
                return *this;
            }

            auto setQueuesPriority(const vot::vector<float>& p){
                this->priorities = p;
                return *this;
            }

        private:
            template<typename T>
            auto& addExOrLay(const T& extensions, auto& target){
                if constexpr (std::is_same_v<T, const char*> || std::is_array_v<T>){
                    target.emplace_back(extensions);
                } else if constexpr (std::is_same_v<T, vot::vector<const char*>>){
                    target.insert(target.end(), extensions.begin(), extensions.end());
                }
                return *this;
            }

            template<typename feature>
            bool addFeatureToChain(feature* f){
                f->pNext = features2.pNext;
                features2.pNext = f;

                return true;
            }

        public:
            vk::PhysicalDeviceFeatures2 features2{};
            vot::vector<float> priorities{1.f};
            vot::vector<const char*> instanceExtensions{},
                    instanceLayers{},
                    physicalExtensions{};
        };
    public:
        explicit VkInit(CreateInfo createInfo);
        ~VkInit();

    private:
        auto createInstance() -> vk::Instance;
        auto createDebugMessenger() -> vk::DebugUtilsMessengerEXT;
        auto pickPhysicalDevice() -> vk::PhysicalDevice;
        auto createLogicalDevice() -> vk::Device;

    private:
        auto addRequiredExtensions() -> vot::vector<const char*>;
        template<typename vkProperties, typename NameFunc>
        auto checkSupport(const vkProperties& properties, vot::vector<const char*> requiredExs, NameFunc name, bool print = false) -> bool{
            vot::vector<const char*> foundExs;
            for(auto& pro : properties){
                auto it = std::find_if(requiredExs.begin(), requiredExs.end(), [&](const char* requiredEx){
                    return strcmp(name(pro), requiredEx) == 0;
                });
                if (it != requiredExs.end()){
                    if_debug{ yic::logger->warn("Enable extension or layer : {0}", static_cast<const char*>(name(pro)));}
                    foundExs.emplace_back(*it);
                } else{
                    if_debug{ if (print) { yic::logger->trace(static_cast<const char*>( name(pro)));}}
                }
            }

            for(auto& found : foundExs){
                requiredExs.erase(std::remove(requiredExs.begin(), requiredExs.end(), found), requiredExs.end());
            }

            if (!requiredExs.empty()){
                if_debug{ yic::logger->info("the following required items were not found: "); }
                for (const auto& missingEx : requiredExs) {
                    if_debug{ yic::logger->error(static_cast<const char*> (missingEx)); }
                }
                return false;
            }

            return true;
        }
        auto checkInstanceSupport(const vot::vector<const char*>& extensions, const vot::vector<const char*>& layers, bool print = false) -> bool;
        auto checkPhysicalSupport(const vk::PhysicalDevice& physicalDevice, const vot::vector<const char*>& extensions, bool print = false) -> bool;
    private:
        CreateInfo mCreateInfo{};
        vk::Instance mInstance{};
        vk::DispatchLoaderDynamic mDynamicDispatcher{};
        vk::DebugUtilsMessengerEXT mDebugMessenger{};
        vk::PhysicalDevice mPhysicalDevice{};
        vk::Device mDevice{};
    };


} // rhi

#endif //VKCELSHADINGRENDERER_VKINIT_H
