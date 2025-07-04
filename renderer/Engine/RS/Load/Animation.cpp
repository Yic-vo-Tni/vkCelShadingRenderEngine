//
// Created by lenovo on 5/23/2025.
//

#include "Animation.h"



namespace rs {

    Animation::Animation(const aiAnimation* aiAnim, vot::AnimationComponent &ac) {
        mDuration = static_cast<float>(aiAnim->mDuration);
        mTicksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond);
        readMissingBones(aiAnim, ac);
    }

    Animation::Animation(const std::shared_ptr<vmd::VmdMotion>& vmd, vot::AnimationComponent& ac) {
        auto jp_to_utf8 = [](const std::string& sjis) -> vot::string { return boost::locale::conv::to_utf<char>(sjis, "Shift-JIS").c_str(); };
//        auto vmd = vmd::VmdMotion::LoadFromFile(pt.c_str());
        mDuration = 0.f;
        mTicksPerSecond = 30.f;

        vot::unordered_map<vot::string, vot::vector<vmd::VmdBoneFrame>> channels;

        for(auto& f : vmd->bone_frames){
            channels[jp_to_utf8(f.name)].emplace_back(f);
        }

        int maxFrame{0};
        for(auto& [bone, frames] : channels){
            std::ranges::sort(frames, [](vmd::VmdBoneFrame& a, vmd::VmdBoneFrame& b){ return a.frame < b.frame; });
            maxFrame = std::max(maxFrame, frames.back().frame);
        }
        mDuration = (float)maxFrame;

        yic::logger->warn("duration {0}", mDuration);

        auto& boneMap = ac.boneMap;
        auto& boneCount = ac.boneCount;
        for(auto channel : channels){
           auto boneName = channel.first;

            if (boneMap.find(boneName) == boneMap.end()){
                yic::logger->info("find bone name {0}", boneName);
                boneMap[boneName].id = boneCount;
                boneCount++;
            }

            mBones.emplace_back(boneName, boneMap[boneName].id, channel.second);
        }

        yic::logger->warn("add success");
    }

    Animation::~Animation() = default;

    auto Animation::readMissingBones(const aiAnimation *aiAnim, vot::AnimationComponent &ac) -> void {
        auto& boneMap = ac.boneMap;
        auto& boneCount = ac.boneCount;

        for(auto channel : std::span<aiNodeAnim*>(aiAnim->mChannels, aiAnim->mNumChannels)){
            auto boneName = channel->mNodeName.data;

            if (boneMap.find(boneName) == boneMap.end()){
                boneMap[boneName].id = boneCount;
                boneCount++;
            }

            mBones.emplace_back(boneName, boneMap[boneName].id, channel);
        }
    }

    auto Animation::findBone(const vot::string &name) -> Bone * {
        auto item = std::ranges::find_if(mBones, [&](const Bone& bone){
            return bone.GetBoneName() == name;
        });
        if (item == mBones.end()) return nullptr;
        else return &(*item);
    }


} // rs