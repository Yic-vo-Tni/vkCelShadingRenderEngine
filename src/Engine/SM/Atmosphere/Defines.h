//
// Created by lenovo on 9/21/2025.
//

#ifndef VKCELSHADINGRENDERER_DEFINES_H
#define VKCELSHADINGRENDERER_DEFINES_H

namespace sm {
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec4 time_3pad;
        glm::mat4 lHViewProj;
    };

    struct PostProcessParamsBuffer {
        union {
            glm::vec4 minimumLuminance_maximumLuminance_timeDelta_lumAdapTau;

            struct {
                float minimumLuminance;
                float maximumLuminance;
                float timeDelta;
                float lumAdaptTau;
            };
        };

        union {
            glm::vec4 whitePoint_maxDisplayBrightness_contrast_linearSectionStart;

            struct {
                float whitePoint;
                float maxDisplayBrightness;
                float contrast;
                float linearSectionStart;
            };
        };

        union {
            glm::vec4 linearSectionLength_black_pedestal_a;

            struct {
                float linearSectionLength;
                float black;
                float pedestal;
                float a;
            };
        };

        union {
            glm::vec4 d_hdrMax_midIn_midOut;

            struct {
                float d;
                float hdrMax;
                float midIn;
                float midOut;
            };
        };

        union {
            glm::vec4 toneMapCurve_2texDimensions_1pad;

            struct {
                float toneMapCurve;
                glm::vec2 texDimensions;
                float _pad;
            };
        };
    };


    struct CloudsParametersBuffer {
        glm::vec4 shapeNoiseWeights;
        glm::vec4 detailNoiseWeights;

        union {
            glm::vec4 detailNoiseMultiplier_minBounds_maxBounds_cloudsScale;

            struct {
                float detailNoiseMultiplier;
                float minBounds;
                float maxBounds;
                float cloudsScale;
            };
        };

        union {
            glm::vec4 detailScale_densityOffset_densityMultiplier_sampleCount;

            struct {
                float detailScale;
                float densityOffset;
                float densityMultiplier;
                float sampleCount;
            };
        };

        union {
            glm::vec4 sampleCountToSun_lightAbsTowardsSun_lightAbsThroughCloud_darknessThreshold;

            struct {
                float sampleCountToSun;
                float lightAbsTowardsSun;
                float lightAbsThroughCloud;
                float darknessThreshold;
            };
        };

        union {
            glm::vec4 debug_pad123;

            struct {
                float debug;
                float _pad[3];
            };
        };

        glm::vec4 phaseParams;
    };
}

#endif //VKCELSHADINGRENDERER_DEFINES_H
