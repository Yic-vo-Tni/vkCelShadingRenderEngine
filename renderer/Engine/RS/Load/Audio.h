//
// Created by lenovo on 6/17/2025.
//

#ifndef VKCELSHADINGRENDERER_AUDIO_H
#define VKCELSHADINGRENDERER_AUDIO_H

#include "miniaudio/miniaudio.h"

namespace vot{
    template<typename key, typename value, typename hash = std::hash<key>, typename keyEq = std::equal_to<key>>
            class ordered_map{
            public:
                ordered_map() = default;
                ~ordered_map() { clear(); }

                value& ensure(const key& k) {
                    auto it = map.find(k);
                    if(it == map.end()) {
                        order.push_back(k);
                        auto res = map.emplace(k, value{});
                        it = res.first;
                    }
                    return it->second;
                }

                auto insert(const key& k, value&& v) -> void{
                    auto it = map.find(k);
                    if (it == map.end()){
                        order.emplace_back(k);
                        auto [ke, ok] = map.emplace(k, std::move(v));
                    } else {
                        it->second = std::move(v);
                    }
                }

                auto find(const key& k) -> value*{
                    auto it = map.find(k);
                    return it != map.end() ? &it->second : nullptr;
                }
                auto find(const key& k) const -> const value*{
                    auto it = map.find(k);
                    return it != map.end() ? &it->second : nullptr;
                }

                template<typename fn>
                auto forEach(fn&& f) const {
                    for(auto const& k : order){
                        fn(k, map.at(k));
                    }
                }

                void clear() {
                    map.clear();
                    order.clear();
                }

                [[nodiscard]] size_t size() const { return order.size(); }
                [[nodiscard]] bool empty() const { return order.empty(); }

            private:
                vot::vector<key> order;
                vot::unordered_map<key, value, hash, keyEq> map;
            };
}


namespace rs {

    class Audio {
    public:
        Audio();
        ~Audio();

        auto Load(const std::filesystem::path& pt) -> void;
        auto play() -> void;
        auto stop() -> void;
        auto seek(float seconds) -> void;
//        auto pos() const -> float;

    public:
//        auto& gSounds() { return mSounds; }
    private:
        ma_engine mAudioEngine{};

        vot::string mActiveSound{};
        vot::unordered_map<vot::string, std::unique_ptr<ma_sound>> mSounds;
        uint8_t handle = 0;
    };

} // rs

#endif //VKCELSHADINGRENDERER_AUDIO_H
