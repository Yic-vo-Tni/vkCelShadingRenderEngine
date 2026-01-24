//
// Created by lenovo on 9/21/2025.
//

#include "Atmosphere.h"

#define SET0  addDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR) \
.addDescriptorSetLayoutBinding(0, 1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment) \

namespace sm {
    Atmosphere::Atmosphere() {
        postProcessParameters = {
            .minimumLuminance_maximumLuminance_timeDelta_lumAdapTau = {100.0f, 6000.0f, 0.0f, 1.1f},
            .whitePoint_maxDisplayBrightness_contrast_linearSectionStart = {4.0f, 1.0f, 1.0f, 0.22f},
            .linearSectionLength_black_pedestal_a = {0.4f, 1.33f, 0.0f, 1.6f},
            .d_hdrMax_midIn_midOut = {0.977f, 8.0f, 0.18f, 0.267f},
            .toneMapCurve_2texDimensions_1pad = {4, glm::vec2(0.0f, 0.0f), 0.0f}
        };

        cloudsParameters = {
            .shapeNoiseWeights = {0.854f, 0.130f, 0.076f, 0.515f},
            .detailNoiseWeights = {0.f, 1.f, 0.5f, 0.5f},
            .detailNoiseMultiplier_minBounds_maxBounds_cloudsScale = {0.329f, 4.f, 8.306f, 0.3f},
            .detailScale_densityOffset_densityMultiplier_sampleCount = {6.645f, 0.688f, 0.269f, 100.f},
            .sampleCountToSun_lightAbsTowardsSun_lightAbsThroughCloud_darknessThreshold = {4.f, 3.123f, 0.100f, 0.093f},
            .debug_pad123 = {1.f, 0.f, 0.f, 0.f},
            .phaseParams = {0.52f, 0.52f, 0.700f, 0.100f}
        };

        createPreset();
        createPipeline();
    }

    auto Atmosphere::createPreset() -> void {
        cloudsParameters.minBounds = 5.720; // 云层底部 .km
        cloudsParameters.maxBounds = 8.0f;  // 高度
        cloudsParameters.cloudsScale = 1.396; // 基础噪声缩放， 云团大小，数值越小，云块越大，数值越大，云更碎，更有细节
        cloudsParameters.detailScale = 8.911; // 细节噪声缩放。 高值 → 云看起来毛茸茸；低值 → 云看起来光滑。
        cloudsParameters.shapeNoiseWeights = glm::vec4(0.559, 0.272, 0.125f, 0.158f); // 基础 Worley 噪声的权重。大的分量决定云的整体形状，小的分量加一点点细节。
        cloudsParameters.detailNoiseWeights = glm::vec4(0.767f, 0.262f, 0.168f, 0.277f); // 细节 Worley 噪声的权重。 让云边缘更毛、内部更蓬松。
        cloudsParameters.densityOffset = 0.817f; // 密度偏移。控制云“起始的稠密度”。数值高 → 云整体更厚，容易变成乌云。太低，云可能几乎透明。
        cloudsParameters.detailNoiseMultiplier = 0.329f; //细节噪声强度。高 → 云边缘破碎感强，低 → 云更圆滑。
        cloudsParameters.sampleCount = 60.f; //raymarch 采样点数量
        cloudsParameters.sampleCountToSun = 5.f; //二次采样（朝太阳方向）。数字越大，太阳边缘的“银边云”效果越真实。
        cloudsParameters.lightAbsTowardsSun = 0.248f; //光在朝太阳方向传播时的吸收率。低值 → 云更透亮；高值 → 云更容易黑。
        cloudsParameters.lightAbsThroughCloud = 0.238f;  //光垂直穿透云层的吸收率。是“从眼睛 → 太阳方向”而不是“太阳 → 云内部”。决定云层整体是亮白还是灰暗
        cloudsParameters.darknessThreshold = 0.238f;  //暗部阈值。值低 → 云内部容易过黑；值高 → 云内部会留一点亮度，看起来更柔和。
        cloudsParameters.debug = 1.f;  //
        cloudsParameters.phaseParams = glm::vec4(0.863f, -0.528f, 1.676f, 0.216f); //相函数参数（Phase Function）。决定云有没有“银边”（强前向散射）


        // NOTE

        postProcessParameters.minimumLuminance = 100.f; //最低亮度（cd/m²），防止画面太黑。
        postProcessParameters.maximumLuminance = 6000.f; //最高亮度（cd/m²），模拟 HDR 显示器的峰值亮度。
        postProcessParameters.lumAdaptTau = 1.1f; //亮度适应速度 数字越小 → 适应快（闪瞎眼 → 立刻恢复）；数字越大 → 适应慢（逐渐变亮/变暗）。
        postProcessParameters.toneMapCurve = 4.f; //使用的 tone mapping 曲线类型（枚举/索引）。1 = Reinhard 2 = Uchimura 3 = Lottes 4 = Hable (Uncharted 2 curve) 或自定义
        postProcessParameters.whitePoint = 4.f; //Reinhard/Uchimura 曲线里的 白点。决定多亮才算“白”，影响高光压缩效果。
        postProcessParameters.maxDisplayBrightness = 1.f; //Uchimura 曲线参数：显示器最大亮度（归一化后）。决定 tone mapping 的映射范围。
        postProcessParameters.contrast = 1.0f; //对比度系数。1.0 = 正常，>1 = 提高对比度，<1 = 降低对比度。
        postProcessParameters.linearSectionLength = 0.4f; //Uchimura/Lottes 曲线参数：线性段的长度。数字大 → 暗部更亮（灰蒙蒙）；数字小 → 暗部压黑更快。
        postProcessParameters.black = 1.33f; //Lottes 曲线参数：黑电平调整。1 → 抬高暗部（变灰）；<1 → 压低暗部（更黑）。
        postProcessParameters.pedestal = 0.f; //黑电平偏移（通常加在整个曲线上）。用于抬高整个画面亮度。
        postProcessParameters.a = 1.6f; //Lottes 曲线参数：高光 roll-off 控制。数字越大 → 高光压缩更强，避免过曝。
        postProcessParameters.d = 0.977f; //Lottes 曲线参数：暗部曲率。控制阴影区域的渐变，接近 1 → 比较线性。
        postProcessParameters.hdrMax = 8.f; //Lottes 曲线参数：输入 HDR 最大亮度。决定多亮的输入会被压到输出白点。
        postProcessParameters.midIn = 0.18f; //中间调输入（18% 灰）。HDR 输入中“正常亮度”的参考点。
        postProcessParameters.midOut = 0.267f; //中间调输出（映射后的亮度）。决定 18% 灰在屏幕上显示成多亮。用来控制整体曝光。
    }

    auto Atmosphere::loadAssets() -> void {
        // NOTE : 地形
    }

    auto Atmosphere::createPipeline() -> void {
        // CP_TransmittanceLUT.build("PreAtmosphere/transmittanceLUT", vot::PipelineDescriptorSetLayoutCI2()
        //     .SET0
        //     // TODO: commonUbo, skyConstantUbo, computerLUTTexs
        //     );
        //
        // CP_SkyViewLUT.build("PreAtmosphere/skyViewLUT", vot::PipelineDescriptorSetLayoutCI2()
        //     .SET0
        //     // TODO: commonUbo, skyConstantUbo, computeLutTexs
        //     );
        //
        // GP_Clouds.combinePipelineLibrary(vot::PipelineLibrary()
        //     .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
        //     .SET0
        //     // TODO: commonUbo, skyConstantUbo, CloudsParamsUbo, DepthOne, worleyNoise, TransmittanceLUT,
        //     )
        //
        //     .setRenderPass2CI(vot::RenderPass2CI()
        //     .setRenderingDepth(vk::True))
        //
        //     .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
        //     .setShaderPath("Common/screen_triangle.vert"))
        //
        //     .setFragmentShaderCI(vot::FragmentShaderCI()
        //     .setShaderPath("PreAtmosphere/clouds.frag")));
        //
        // GP_FarSky.combinePipelineLibrary(vot::PipelineLibrary()
        //     .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
        //     .SET0
        //     // TODO: commonUbo, skyConstantUbo, SkyViewLUT, DepthOne
        //     )
        //
        //     .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
        //     .setShaderPath("Common/screen_triangle.vert"))
        //
        //     .setFragmentShaderCI(vot::FragmentShaderCI() //NOTE: blendAlpha maybe should 0
        //     .setShaderPath("PreAtmosphere/farSky.frag")));
    }
} // sm