//
// Created by lenovo on 6/17/2025.
//

#include "Audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

namespace rs {

    Audio::Audio() {
        if (ma_engine_init(nullptr, &mAudioEngine) != MA_SUCCESS){
            yic::logger->error("failed to init audio engine");
        }
    }

    Audio::~Audio() {
//        if (&music){
//            ma_sound_uninit(&music);
//        }
        if (!mSounds.empty()){
            mSounds.clear();
        }

        ma_engine_uninit(&mAudioEngine);
    };

    auto Audio::Load(const std::filesystem::path &pt) -> void {
        ma_uint32 flags = MA_SOUND_FLAG_DECODE;

        vot::string key = pt.stem().string().c_str();
        auto it = mSounds.find(key);
        if(it != mSounds.end()){
            ma_sound_uninit(it->second.get());
            mSounds.erase(it);
        }

        auto ptr = std::make_unique<ma_sound >();

        if (ma_sound_init_from_file(&mAudioEngine, pt.string().c_str(), flags, nullptr, nullptr, ptr.get()) != MA_SUCCESS){
            yic::logger->warn("failed to load audio");
        }

        mSounds.emplace(key, std::move(ptr));

        if (mActiveSound.empty())
            mActiveSound = key;
    }

    auto Audio::play() -> void {
        if (!mActiveSound.empty() && handle == 0){
            ma_sound_start(mSounds.find(mActiveSound)->second.get());
            handle = 1;
        }
    }

    auto Audio::stop() -> void {
        ma_sound_stop(mSounds.find(mActiveSound)->second.get());
     //   ma_sound_seek_to_pcm_frame(&music, 0);
    }

    auto Audio::seek(float seconds) -> void {
            auto frame = (ma_uint64)(seconds * (float)mAudioEngine.sampleRate);
            ma_sound_seek_to_pcm_frame(mSounds.find(mActiveSound)->second.get(), frame);
    }

//    auto Audio::pos() const -> float {
//        ma_uint64 frame = ma_sound_get_cursor_in_pcm_frames(&music, 0);
//        return float(frame) / float(music.pSampleRate);
//    }

} // rs