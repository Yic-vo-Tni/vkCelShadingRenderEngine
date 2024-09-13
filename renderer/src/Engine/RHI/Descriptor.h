//
// Created by lenovo on 7/8/2024.
//

#ifndef VKCELSHADINGRENDERER_DESCRIPTOR_H
#define VKCELSHADINGRENDERER_DESCRIPTOR_H

#include <utility>

#include "Engine/RHI/vkCommon.h"
#include "Engine/RHI/vkAsset.h"

#include "Engine/Utils/Log.h"

namespace yic {

    struct FixSampler{
        vkGet auto get = []{ return Singleton<FixSampler>::get();};
        FixSampler();
        ~FixSampler();
        static auto createSampler() ->  vk::Sampler;

        static vk::Sampler eDefault;
    };

    struct ImgInfo{
        vk::Sampler sampler{FixSampler::eDefault};
        std::vector<vk::ImageView> imageViews;
        vk::ImageLayout imageLayout{vk::ImageLayout::eShaderReadOnlyOptimal};

        explicit ImgInfo(const std::vector<vk::ImageView>& imageViews) : imageViews(imageViews){}
        ImgInfo(const std::vector<vk::ImageView>& imageViews, vk::ImageLayout imageLayout) : imageViews(imageViews), imageLayout(imageLayout){}
        ImgInfo(vk::Sampler sampler, const std::vector<vk::ImageView>& imageViews) : sampler(sampler), imageViews(imageViews){}
        ImgInfo(vk::Sampler sampler, const std::vector<vk::ImageView>& imageViews, vk::ImageLayout imageLayout) : sampler(sampler), imageViews(imageViews), imageLayout(imageLayout){}
        ImgInfo(vk::Sampler sampler, const std::shared_ptr<yic::vkImage>& img, vk::ImageLayout imageLayout) : sampler(sampler), imageViews(img->imageViews), imageLayout(imageLayout){}
        ImgInfo(vk::Sampler sampler, const std::shared_ptr<yic2::Image>& img, vk::ImageLayout imageLayout) : sampler(sampler), imageLayout(imageLayout){
            for(auto& v : img->imageViews){
                imageViews.emplace_back(v);
            }
        }

        explicit ImgInfo(const std::vector<std::shared_ptr<yic::vkImage>>& imgs){
            for(const auto& img : imgs){
                imageViews.emplace_back(img->imageViews.back());
            }
        }
        explicit ImgInfo(const std::shared_ptr<yic::vkImage>& img){
            imageViews.insert(imageViews.end(), img->imageViews.begin(), img->imageViews.end());
        }
        explicit ImgInfo(const yic2::Image_sptr& img){
            imageViews.insert(imageViews.end(), img->imageViews.begin(), img->imageViews.end());
        }


    };
    struct BufInfo{
        std::vector<vk::Buffer> buffer;
        std::vector<vk::DeviceSize> offset{0};
        std::vector<vk::DeviceSize> range{VK_WHOLE_SIZE};

        explicit BufInfo(vk::Buffer buffer) : buffer(std::vector<vk::Buffer>{buffer}){}
        explicit BufInfo(const std::shared_ptr<vkBuffer>& buffer) : buffer(std::vector<vk::Buffer>{buffer->buffer}){}
        explicit BufInfo(const std::vector<vk::Buffer>& buffer) : buffer(buffer){}
        explicit BufInfo(const std::vector<std::shared_ptr<vkBuffer>>& buffer) {
            for(auto& buf : buffer){
                this->buffer.emplace_back(buf->buffer);
            }
        }
    };

    struct AccelInfo{
        std::vector<vk::AccelerationStructureKHR> accel;

        explicit AccelInfo(const vk::AccelerationStructureKHR& accel) : accel( {accel} ) {}
        explicit AccelInfo(const std::vector<vk::AccelerationStructureKHR>& accel) : accel(accel){}
        explicit AccelInfo(const std::shared_ptr<vkAccel>& accel) : accel({accel->accel}){}

    };

//    template<typename T>
//    concept DescriptorVariantType = std::same_as<T, ImgInfo> ||
//                                    std::same_as<T, BufInfo> ||
//                                    std::same_as<T, AccelInfo> ||
//                                    std::same_as<T, vk::Buffer> ||
//                                    std::same_as<T, std::shared_ptr<vkBuffer>> ||
//                                    std::same_as<T, std::vector<vk::Buffer>> ||
//                                    std::same_as<T, std::vector<std::shared_ptr<vkBuffer>>> ||
//                                    std::same_as<T, std::vector<vk::ImageView>> ||
//                                    std::same_as<T, std::shared_ptr<yic::vkImage>> ||
//                                    std::same_as<T, std::vector<std::shared_ptr<yic::vkImage>>> ||
//                                    std::same_as<T, std::vector<vk::AccelerationStructureKHR>> ||
//                                    std::same_as<T, vk::AccelerationStructureKHR>;


    class Descriptor : public Identifiable, public std::enable_shared_from_this<Descriptor>{
        using descriptorInfo = std::variant<ImgInfo, BufInfo, AccelInfo>;
    public:
        explicit Descriptor(const std::string& id, PipelineDesSetLayout& setLayout);
        ~Descriptor() override{
            if (mDesPool) {
                mDevice.destroyDescriptorPool(mDesPool);
            }
        }

        static auto configure(PipelineDesSetLayout& setLayout) -> std::shared_ptr<Descriptor> {
            return std::make_shared<Descriptor>(IdGenerator::uniqueId(), setLayout);
        }

        template<typename T>
        static auto configure(std::shared_ptr<T>& setLayout) -> std::shared_ptr<Descriptor>{
            return std::make_shared<Descriptor>(IdGenerator::uniqueId(), *setLayout);
        }

        [[nodiscard]] inline auto& getDescriptorSets() const { return mDesSet;}

        Descriptor& createDesPool(std::optional<uint32_t> Reset_MaxSets = std::nullopt);
        Descriptor& pushbackDesSets(uint32_t setIndex = 0);
        Descriptor& updateDesSet(const std::vector<std::variant<vk::DescriptorBufferInfo,
                                 vk::DescriptorImageInfo, vk::WriteDescriptorSetAccelerationStructureKHR>> &infos, const size_t& setIndex = 0);
        Descriptor& updateDesSet(uint32_t Reset_MaxSets, const std::vector<std::variant<ImgInfo, BufInfo, AccelInfo>> &infos, const size_t& setIndex = 0);

        template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
        template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
        std::shared_ptr<Descriptor> updateDesSet(const std::vector<std::variant<ImgInfo, BufInfo, AccelInfo>> &infos, const size_t& setIndex = 0){
            size_t maxSize = 0;
            for(const auto& info : infos){
                std::visit(overloaded{
                    [&](const ImgInfo& imgInfo){
                        maxSize = std::max(maxSize, imgInfo.imageViews.size());
                    },
                    [&](const BufInfo& bufInfo){
                        maxSize = std::max(maxSize, bufInfo.buffer.size());
                    },
                    [&](const AccelInfo& accelInfo){
                        maxSize = std::max(maxSize, accelInfo.accel.size());
                    },
                }, info);
            }

            updateDesSet(maxSize, infos, setIndex);
            return shared_from_this();
        };


        template<typename ...Args>
        //   requires (DescriptorVariantType<Args> && ...)
        std::shared_ptr<Descriptor> updateDesSetAuto(Args &&...args) {
            std::vector<descriptorInfo> infos;
            infos.reserve(sizeof...(Args));

            auto addInfo = [&infos](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                static_assert(std::is_same_v<vk::Buffer, T> ||
                              std::is_same_v<std::shared_ptr<vkBuffer>, T> ||
                              std::is_same_v<std::vector<vk::Buffer>, T> ||
                              std::is_same_v<std::vector<std::shared_ptr<vkBuffer>>, T> ||
                              std::is_same_v<std::vector<vk::ImageView>, T> ||
                              std::is_same_v<std::shared_ptr<yic::vkImage>, T> ||
                              std::is_same_v<std::vector<std::shared_ptr<vkImage>>, T> ||
                              std::is_same_v<std::vector<vk::AccelerationStructureKHR>, T> ||
                              std::is_same_v<vk::AccelerationStructureKHR, T> ||
                              std::is_same_v<std::shared_ptr<yic::vkAccel>, T> ||
                              std::is_same_v<ImgInfo, T> ||
                              std::is_same_v<BufInfo, T> ||
                              std::is_same_v<AccelInfo, T>,
                              "Invalid type passed to updateDesSet");
                try {
                    if constexpr (std::is_same_v<vk::Buffer, T> || std::is_same_v<std::shared_ptr<vkBuffer>, T>
                                  || std::is_same_v<std::vector<vk::Buffer>, T> ||
                                  std::is_same_v<std::vector<std::shared_ptr<vkBuffer>>, T>) {
                        infos.emplace_back(BufInfo{std::forward<decltype(arg)>(arg)});
                    } else if constexpr (std::is_same_v<std::vector<vk::ImageView>, T> ||
                                         std::is_same_v<std::shared_ptr<yic::vkImage>, T>
                                         || std::is_same_v<std::vector<std::shared_ptr<vkImage>>, T>) {
                        infos.emplace_back(ImgInfo{std::forward<decltype(arg)>(arg)});
                    } else if constexpr (std::is_same_v<std::vector<vk::AccelerationStructureKHR>, T> ||
                                         std::is_same_v<vk::AccelerationStructureKHR, T> ||
                                         std::is_same_v<std::shared_ptr<yic::vkAccel>, T>) {
                        infos.emplace_back(AccelInfo{std::forward<decltype(arg)>(arg)});
                    } else if constexpr (std::is_same_v<ImgInfo, T> || std::is_same_v<BufInfo, T> ||
                                         std::is_same_v<AccelInfo, T>) {
                        infos.emplace_back(std::forward<decltype(arg)>(arg));
                    }
                } catch (const std::exception &e) {
                    throw std::runtime_error("invalid type passed to updateDesSet");
                }
            };

            (addInfo(std::forward<Args>(args)), ...);

            updateDesSet(infos);

            return shared_from_this();
        }

        template<typename ...Args>
        Descriptor& updateDesSetAuto(uint32_t setIndex, Args&&...args){
            std::vector<descriptorInfo > infos;
            infos.reserve(sizeof...(Args));

            auto addInfo = [&infos](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_constructible<descriptorInfo, T>::value) {
                    infos.emplace_back(std::forward<decltype(arg)>(arg));
                } else {
                    throw std::runtime_error("Invalid type passed to updateDesSet");
                }
            };

            (addInfo(std::forward<Args>(args)), ...);

            updateDesSet(infos, setIndex);

            return *this;
        }

        auto getSetLayout() { return mSetLayout; }

    private:
        vk::Device mDevice;

        uint32_t mIndex{};
        vk::DescriptorPool mDesPool{};
        PipelineDesSetLayout& mSetLayout;
        vot::vector<vk::DescriptorSet> mDesSet{};
        vot::vector<vot::vector<vk::WriteDescriptorSet>> mWriteDesSets{};
    };

    class ImGuiDescriptorManager{
    public:
        vkGet auto get = []{ return Singleton<ImGuiDescriptorManager>::get(); };
        ImGuiDescriptorManager();
        ~ImGuiDescriptorManager();

        static auto updateImage(const std::string& id, const std::vector<vk::ImageView>& views) -> void;
        static auto drawImage(const std::string& id, const ImVec2& imageSize, const uint32_t& index = UINT32_MAX) -> void;
        static auto clear() { get()->mSetLayout.reset(); }
    private:
        std::shared_ptr<PipelineDesSetLayout> mSetLayout;
    };

    using Descriptor_sptr = std::shared_ptr<Descriptor>;
    using ImageInfo = ImgInfo;
    using BufferInfo = BufInfo;
} // yic

namespace Hide{
    class ImGuiDescriptorManager{
    public:
        ImGuiDescriptorManager();
        ~ImGuiDescriptorManager();

//        auto updateImage(const std::string& id, const std::vector<vk::ImageView>& views) -> void;
//        auto drawImage(const std::string& id, const ImVec2& imageSize, const uint32_t& index = UINT32_MAX) -> void;
//        auto clear() { mSetLayout.reset(); }
    private:
        std::shared_ptr<PipelineDesSetLayout> mSetLayout;
    };
}

//namespace mg{
//    inline auto& ImGuiDescriptorHub = Singleton<Hide::ImGuiDescriptorManager>::ref();
//}




#endif //VKCELSHADINGRENDERER_DESCRIPTOR_H
