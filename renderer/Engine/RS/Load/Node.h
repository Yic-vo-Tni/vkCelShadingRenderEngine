//
// Created by lenovo on 6/11/2025.
//

#ifndef VKCELSHADINGRENDERER_NODE_H
#define VKCELSHADINGRENDERER_NODE_H

#include <utility>

#include "RHI/Allocator.h"

namespace rs{
    class AssimpGLMHelpers
    {
    public:

        static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
        {
            glm::mat4 to;
            to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
            to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
            to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
            to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
            return to;
        }

        static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
        {
            return glm::vec3(vec.x, vec.y, vec.z);
        }

        static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
        {
            return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
        }
    };

    struct KeyPosition
    {
        glm::vec3 position;
        float timeStamp;
    };

    struct KeyRotation
    {
        glm::quat orientation;
        float timeStamp;
    };

    struct KeyScale
    {
        glm::vec3 scale;
        float timeStamp;
    };

    class Bone {
    public:
        Bone(vot::string name, int ID, const vot::vector<vmd::VmdBoneFrame> &frames)
                : m_Name(std::move(name)), m_ID(ID),
                  m_LocalTransform(1.f) {
            m_NumPositions = m_NumRotations = m_NumScalings = (int)frames.size();
            yic::logger->warn("frame size {0}", m_NumScalings);

            for(auto& f : frames){
                float t = float(f.frame) / 30.f;

                KeyPosition kp{
                    .position = glm::vec3(f.position[0], f.position[1], f.position[2]),
                    .timeStamp = t,
                };
                KeyRotation kr{
                    .orientation = glm::normalize(
                            glm::quat(f.orientation[3], f.orientation[0],
                                      f.orientation[1], f.orientation[2])),
                    .timeStamp = t,
                };
                KeyScale ks{
                    .scale = glm::vec3 (1.f),
                    .timeStamp = t,
                };

                m_Positions.emplace_back(kp);
                m_Rotations.emplace_back(kr);
                m_Scales.emplace_back(ks);
            }
        }


        Bone(vot::string name, int ID, const aiNodeAnim *channel)
                : m_Name(std::move(name)),
                  m_ID(ID),
                  m_LocalTransform(1.0f) {

            m_NumPositions = channel->mNumPositionKeys;
            for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
                aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
                float timeStamp = channel->mPositionKeys[positionIndex].mTime;
                KeyPosition data{};
                data.position = glm::vec3 (aiPosition.x, aiPosition.y, aiPosition.z);
                data.timeStamp = timeStamp;
                m_Positions.push_back(data);
            }

            m_NumRotations = channel->mNumRotationKeys;
            for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
                aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
                float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
                KeyRotation data{};
                data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
                data.timeStamp = timeStamp;
                m_Rotations.push_back(data);
            }

            m_NumScalings = channel->mNumScalingKeys;
            for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
                aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
                float timeStamp = channel->mScalingKeys[keyIndex].mTime;
                KeyScale data{};
                data.scale = AssimpGLMHelpers::GetGLMVec(scale);
                data.timeStamp = timeStamp;
                m_Scales.push_back(data);
            }
        }

        void Update(float animationTime) {
            glm::mat4 translation = InterpolatePosition(animationTime);
            glm::mat4 rotation = InterpolateRotation(animationTime);
            glm::mat4 scale = InterpolateScaling(animationTime);
            m_LocalTransform = translation * rotation * scale;
        }

        glm::mat4 GetLocalTransform() { return m_LocalTransform; }

        [[nodiscard]] vot::string GetBoneName() const { return m_Name; }

        int GetBoneID() { return m_ID; }


        int GetPositionIndex(float animationTime) {
            for (int index = 0; index < m_NumPositions - 1; ++index) {
                if (animationTime < m_Positions[index + 1].timeStamp)
                    return index;
            }
            assert(0);
            return 0;
        }

        int GetRotationIndex(float animationTime) {
            for (int index = 0; index < m_NumRotations - 1; ++index) {
                if (animationTime < m_Rotations[index + 1].timeStamp)
                    return index;
            }
            assert(0);
            return 0;
        }

        int GetScaleIndex(float animationTime) {
            for (int index = 0; index < m_NumScalings - 1; ++index) {
                if (animationTime < m_Scales[index + 1].timeStamp)
                    return index;
            }
            assert(0);
            return 0;
        }


    private:

        float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
            float scaleFactor = 0.0f;
            float midWayLength = animationTime - lastTimeStamp;
            float framesDiff = nextTimeStamp - lastTimeStamp;
            scaleFactor = midWayLength / framesDiff;
            return scaleFactor;
        }

        glm::mat4 InterpolatePosition(float animationTime) {
            if (1 == m_NumPositions)
                return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

            int p0Index = GetPositionIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
                                               m_Positions[p1Index].timeStamp, animationTime);
            glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position,
                                               scaleFactor);
            return glm::translate(glm::mat4(1.0f), finalPosition);
        }

        glm::mat4 InterpolateRotation(float animationTime) {
            if (1 == m_NumRotations) {
                auto rotation = glm::normalize(m_Rotations[0].orientation);
                return glm::toMat4(rotation);
            }

            int p0Index = GetRotationIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
                                               m_Rotations[p1Index].timeStamp, animationTime);
            glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation,
                                                 scaleFactor);
            finalRotation = glm::normalize(finalRotation);
            return glm::toMat4(finalRotation);

        }

        glm::mat4 InterpolateScaling(float animationTime) {
            if (1 == m_NumScalings)
                return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

            int p0Index = GetScaleIndex(animationTime);
            int p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
                                               m_Scales[p1Index].timeStamp, animationTime);
            glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
            return glm::scale(glm::mat4(1.0f), finalScale);
        }

        vot::vector<KeyPosition> m_Positions;
        vot::vector<KeyRotation> m_Rotations;
        vot::vector<KeyScale> m_Scales;
        int m_NumPositions;
        int m_NumRotations;
        int m_NumScalings;

        glm::mat4 m_LocalTransform;
        vot::string m_Name;
        int m_ID;
    };

//    class Animation
//    {
//    public:
//        Animation() = default;
//
//        Animation(const aiScene* scene, vot::AnimationComponent& ac)
//        {
//            auto animation = scene->mAnimations[0];
//            m_Duration = animation->mDuration;
//            m_TicksPerSecond = animation->mTicksPerSecond;
//            aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
//            globalTransformation = globalTransformation.Inverse();
//            globalInverse = AssimpGLMHelpers::ConvertMatrixToGLMFormat(globalTransformation);
//            ReadMissingBones(animation, ac);
//        }
//
//        ~Animation()
//        {
//        }
//
//        Bone* FindBone(const vot::string& name)
//        {
//            auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
//                                     [&](const Bone& Bone)
//                                     {
//                                         return Bone.GetBoneName() == name;
//                                     }
//            );
//            if (iter == m_Bones.end()) return nullptr;
//            else return &(*iter);
//        }
//
//
//        inline float GetTicksPerSecond() { return m_TicksPerSecond; }
//        inline float GetDuration() { return m_Duration;}
//        inline auto GetGlobalInverse() { return globalInverse; }
//
//    private:
//        void ReadMissingBones(const aiAnimation* animation, vot::AnimationComponent& ac)
//        {
//            int size = animation->mNumChannels;
//
//            auto& boneInfoMap = ac.boneMap;//getting m_BoneInfoMap from Model class
//            int& boneCount = ac.boneCount; //getting the m_BoneCounter from Model class
//
//            //reading channels(bones engaged in an animation and their keyframes)
//            for (int i = 0; i < size; i++)
//            {
//                auto channel = animation->mChannels[i];
//                vot::string boneName = channel->mNodeName.data;
//
//                if (boneInfoMap.find(boneName) == boneInfoMap.end())
//                {
//                    boneInfoMap[boneName].id = boneCount;
//                    boneCount++;
//                }
//                m_Bones.push_back(Bone(channel->mNodeName.data,
//                                       boneInfoMap[channel->mNodeName.data].id, channel));
//            }
//
//            std::cout << "Model bone count = " << ac.boneMap.size()
//                      << ", Animation channels = " << animation->mNumChannels << std::endl;
//            for (auto& [name, info] : ac.boneMap) {
//                std::cout << name << " id=" << info.id
//                          << " offset=\n" << glm::to_string(info.offset) << "\n";
//            }
//
//        }
//
//        float m_Duration;
//        int m_TicksPerSecond;
//        vot::vector<Bone> m_Bones;
//        glm::mat4 globalInverse;
//    };

}

#endif //VKCELSHADINGRENDERER_NODE_H
