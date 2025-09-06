//#include "Model.h"
//
////#include <glad/glad.h>
////
//#include <cassert>
//#include <cmath>
//#include <iostream>
//#include <memory>
//
//#include "constant.h"
//#include "RHI/Allocator.h"
//
//
//namespace atmosphere {
//
//    namespace {
//
//        const char kVertexShader[] = R"(
//    #version 330
//    layout(location = 0) in vec2 vertex;
//    void main() {
//      gl_Position = vec4(vertex, 0.0, 1.0);
//    })";
//
/////
//
//        const char kGeometryShader[] = R"(
//    #version 330
//    layout(triangles) in;
//    layout(triangle_strip, max_vertices = 3) out;
//    uniform int layer;
//    void main() {
//      gl_Position = gl_in[0].gl_Position;
//      gl_Layer = layer;
//      EmitVertex();
//      gl_Position = gl_in[1].gl_Position;
//      gl_Layer = layer;
//      EmitVertex();
//      gl_Position = gl_in[2].gl_Position;
//      gl_Layer = layer;
//      EmitVertex();
//      EndPrimitive();
//    })";
//
//        //
//
//        const char kComputeTransmittanceShader[] = R"(
//    layout(location = 0) out vec3 transmittance;
//    void main() {
//      transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(
//          ATMOSPHERE, gl_FragCoord.xy);
//    })";
//
//        const char kComputeDirectIrradianceShader[] = R"(
//    layout(location = 0) out vec3 delta_irradiance;
//    layout(location = 1) out vec3 irradiance;
//    uniform sampler2D transmittance_texture;
//    void main() {
//      delta_irradiance = ComputeDirectIrradianceTexture(
//          ATMOSPHERE, transmittance_texture, gl_FragCoord.xy);
//      irradiance = vec3(0.0);
//    })";
//
//        const char kComputeSingleScatteringShader[] = R"(
//    layout(location = 0) out vec3 delta_rayleigh;
//    layout(location = 1) out vec3 delta_mie;
//    layout(location = 2) out vec4 scattering;
//    layout(location = 3) out vec3 single_mie_scattering;
//    uniform mat3 luminance_from_radiance;
//    uniform sampler2D transmittance_texture;
//    uniform int layer;
//    void main() {
//      ComputeSingleScatteringTexture(
//          ATMOSPHERE, transmittance_texture, vec3(gl_FragCoord.xy, layer + 0.5),
//          delta_rayleigh, delta_mie);
//      scattering = vec4(luminance_from_radiance * delta_rayleigh.rgb,
//          (luminance_from_radiance * delta_mie).r);
//      single_mie_scattering = luminance_from_radiance * delta_mie;
//    })";
//
//        const char kComputeScatteringDensityShader[] = R"(
//    layout(location = 0) out vec3 scattering_density;
//    uniform sampler2D transmittance_texture;
//    uniform sampler3D single_rayleigh_scattering_texture;
//    uniform sampler3D single_mie_scattering_texture;
//    uniform sampler3D multiple_scattering_texture;
//    uniform sampler2D irradiance_texture;
//    uniform int scattering_order;
//    uniform int layer;
//    void main() {
//      scattering_density = ComputeScatteringDensityTexture(
//          ATMOSPHERE, transmittance_texture, single_rayleigh_scattering_texture,
//          single_mie_scattering_texture, multiple_scattering_texture,
//          irradiance_texture, vec3(gl_FragCoord.xy, layer + 0.5),
//          scattering_order);
//    })";
//
//        const char kComputeIndirectIrradianceShader[] = R"(
//    layout(location = 0) out vec3 delta_irradiance;
//    layout(location = 1) out vec3 irradiance;
//    uniform mat3 luminance_from_radiance;
//    uniform sampler3D single_rayleigh_scattering_texture;
//    uniform sampler3D single_mie_scattering_texture;
//    uniform sampler3D multiple_scattering_texture;
//    uniform int scattering_order;
//    void main() {
//      delta_irradiance = ComputeIndirectIrradianceTexture(
//          ATMOSPHERE, single_rayleigh_scattering_texture,
//          single_mie_scattering_texture, multiple_scattering_texture,
//          gl_FragCoord.xy, scattering_order);
//      irradiance = luminance_from_radiance * delta_irradiance;
//    })";
//
//        const char kComputeMultipleScatteringShader[] = R"(
//    layout(location = 0) out vec3 delta_multiple_scattering;
//    layout(location = 1) out vec4 scattering;
//    uniform mat3 luminance_from_radiance;
//    uniform sampler2D transmittance_texture;
//    uniform sampler3D scattering_density_texture;
//    uniform int layer;
//    void main() {
//      float nu;
//      delta_multiple_scattering = ComputeMultipleScatteringTexture(
//          ATMOSPHERE, transmittance_texture, scattering_density_texture,
//          vec3(gl_FragCoord.xy, layer + 0.5), nu);
//      scattering = vec4(
//          luminance_from_radiance *
//              delta_multiple_scattering.rgb / RayleighPhaseFunction(nu),
//          0.0);
//    })";
//
//////
//
//        const char kAtmosphereShader[] = R"(
//    uniform sampler2D transmittance_texture;
//    uniform sampler3D scattering_texture;
//    uniform sampler3D single_mie_scattering_texture;
//    uniform sampler2D irradiance_texture;
//    #ifdef RADIANCE_API_ENABLED
//    RadianceSpectrum GetSolarRadiance() {
//      return ATMOSPHERE.solar_irradiance /
//          (PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius);
//    }
//    RadianceSpectrum GetSkyRadiance(
//        Position camera, Direction view_ray, Length shadow_length,
//        Direction sun_direction, out DimensionlessSpectrum transmittance) {
//      return GetSkyRadiance(ATMOSPHERE, transmittance_texture,
//          scattering_texture, single_mie_scattering_texture,
//          camera, view_ray, shadow_length, sun_direction, transmittance);
//    }
//    RadianceSpectrum GetSkyRadianceToPoint(
//        Position camera, Position point, Length shadow_length,
//        Direction sun_direction, out DimensionlessSpectrum transmittance) {
//      return GetSkyRadianceToPoint(ATMOSPHERE, transmittance_texture,
//          scattering_texture, single_mie_scattering_texture,
//          camera, point, shadow_length, sun_direction, transmittance);
//    }
//    IrradianceSpectrum GetSunAndSkyIrradiance(
//       Position p, Direction normal, Direction sun_direction,
//       out IrradianceSpectrum sky_irradiance) {
//      return GetSunAndSkyIrradiance(ATMOSPHERE, transmittance_texture,
//          irradiance_texture, p, normal, sun_direction, sky_irradiance);
//    }
//    #endif
//    Luminance3 GetSolarLuminance() {
//      return ATMOSPHERE.solar_irradiance /
//          (PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius) *
//          SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
//    }
//    Luminance3 GetSkyLuminance(
//        Position camera, Direction view_ray, Length shadow_length,
//        Direction sun_direction, out DimensionlessSpectrum transmittance) {
//      return GetSkyRadiance(ATMOSPHERE, transmittance_texture,
//          scattering_texture, single_mie_scattering_texture,
//          camera, view_ray, shadow_length, sun_direction, transmittance) *
//          SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
//    }
//    Luminance3 GetSkyLuminanceToPoint(
//        Position camera, Position point, Length shadow_length,
//        Direction sun_direction, out DimensionlessSpectrum transmittance) {
//      return GetSkyRadianceToPoint(ATMOSPHERE, transmittance_texture,
//          scattering_texture, single_mie_scattering_texture,
//          camera, point, shadow_length, sun_direction, transmittance) *
//          SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
//    }
//    Illuminance3 GetSunAndSkyIlluminance(
//       Position p, Direction normal, Direction sun_direction,
//       out IrradianceSpectrum sky_irradiance) {
//      IrradianceSpectrum sun_irradiance = GetSunAndSkyIrradiance(
//          ATMOSPHERE, transmittance_texture, irradiance_texture, p, normal,
//          sun_direction, sky_irradiance);
//      sky_irradiance *= SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
//      return sun_irradiance * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
//    })";
//
//
//
//
//
//
//
//
//        constexpr int kLambdaMin = 360;
//        constexpr int kLambdaMax = 830;
//
//        double CieColorMatchingFunctionTableValue(double wavelength, int column) {
//            if (wavelength <= kLambdaMin || wavelength >= kLambdaMax) {
//                return 0.0;
//            }
//            double u = (wavelength - kLambdaMin) / 5.0;
//            int row = static_cast<int>(std::floor(u));
//            assert(row >= 0 && row + 1 < 95);
//            assert(CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row] <= wavelength &&
//                   CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1)] >= wavelength);
//            u -= row;
//            return CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row + column] * (1.0 - u) +
//                   CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1) + column] * u;
//        }
//
//        double Interpolate(
//                const std::vector<double>& wavelengths,
//                const std::vector<double>& wavelength_function,
//                double wavelength) {
//            assert(wavelength_function.size() == wavelengths.size());
//            if (wavelength < wavelengths[0]) {
//                return wavelength_function[0];
//            }
//            for (unsigned int i = 0; i < wavelengths.size() - 1; ++i) {
//                if (wavelength < wavelengths[i + 1]) {
//                    double u =
//                            (wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
//                    return
//                            wavelength_function[i] * (1.0 - u) + wavelength_function[i + 1] * u;
//                }
//            }
//            return wavelength_function[wavelength_function.size() - 1];
//        }
//
///*
//<p>We can then implement a utility function to compute the "spectral radiance to
//luminance" conversion constants (see Section 14.3 in <a
//href="https://arxiv.org/pdf/1612.04336.pdf">A Qualitative and Quantitative
//Evaluation of 8 Clear Sky Models</a> for their definitions):
//*/
//
//// The returned constants are in lumen.nm / watt.
//        void ComputeSpectralRadianceToLuminanceFactors(
//                const std::vector<double>& wavelengths,
//                const std::vector<double>& solar_irradiance,
//                double lambda_power, double* k_r, double* k_g, double* k_b) {
//            *k_r = 0.0;
//            *k_g = 0.0;
//            *k_b = 0.0;
//            double solar_r = Interpolate(wavelengths, solar_irradiance, Model::kLambdaR);
//            double solar_g = Interpolate(wavelengths, solar_irradiance, Model::kLambdaG);
//            double solar_b = Interpolate(wavelengths, solar_irradiance, Model::kLambdaB);
//            int dlambda = 1;
//            for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
//                double x_bar = CieColorMatchingFunctionTableValue(lambda, 1);
//                double y_bar = CieColorMatchingFunctionTableValue(lambda, 2);
//                double z_bar = CieColorMatchingFunctionTableValue(lambda, 3);
//                const double* xyz2srgb = XYZ_TO_SRGB;
//                double r_bar =
//                        xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
//                double g_bar =
//                        xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
//                double b_bar =
//                        xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
//                double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);
//                *k_r += r_bar * irradiance / solar_r *
//                        pow(lambda / Model::kLambdaR, lambda_power);
//                *k_g += g_bar * irradiance / solar_g *
//                        pow(lambda / Model::kLambdaG, lambda_power);
//                *k_b += b_bar * irradiance / solar_b *
//                        pow(lambda / Model::kLambdaB, lambda_power);
//            }
//            *k_r *= MAX_LUMINOUS_EFFICACY * dlambda;
//            *k_g *= MAX_LUMINOUS_EFFICACY * dlambda;
//            *k_b *= MAX_LUMINOUS_EFFICACY * dlambda;
//        }
//
//    }  // anonymous namespace
//
//
//
//    Model::Model(
//            const std::vector<double>& wavelengths,
//            const std::vector<double>& solar_irradiance,
//            const double sun_angular_radius,
//            double bottom_radius,
//            double top_radius,
//            const std::vector<DensityProfileLayer>& rayleigh_density,
//            const std::vector<double>& rayleigh_scattering,
//            const std::vector<DensityProfileLayer>& mie_density,
//            const std::vector<double>& mie_scattering,
//            const std::vector<double>& mie_extinction,
//            double mie_phase_function_g,
//            const std::vector<DensityProfileLayer>& absorption_density,
//            const std::vector<double>& absorption_extinction,
//            const std::vector<double>& ground_albedo,
//            double max_sun_zenith_angle,
//            double length_unit_in_meters,
//            unsigned int num_precomputed_wavelengths,
//            bool combine_scattering_textures,
//            bool half_precision) :
//            num_precomputed_wavelengths_(num_precomputed_wavelengths),
//            half_precision_(half_precision){
//        auto to_string = [&wavelengths](const std::vector<double>& v,
//                                        const vec3& lambdas, double scale) {
//            double r = Interpolate(wavelengths, v, lambdas[0]) * scale;
//            double g = Interpolate(wavelengths, v, lambdas[1]) * scale;
//            double b = Interpolate(wavelengths, v, lambdas[2]) * scale;
//            return "vec3(" + std::to_string(r) + "," + std::to_string(g) + "," +
//                   std::to_string(b) + ")";
//        };
//        auto density_layer =
//                [length_unit_in_meters](const DensityProfileLayer& layer) {
//                    return "DensityProfileLayer(" +
//                           std::to_string(layer.width / length_unit_in_meters) + "," +
//                           std::to_string(layer.exp_term) + "," +
//                           std::to_string(layer.exp_scale * length_unit_in_meters) + "," +
//                           std::to_string(layer.linear_term * length_unit_in_meters) + "," +
//                           std::to_string(layer.constant_term) + ")";
//                };
//        auto density_profile =
//                [density_layer](std::vector<DensityProfileLayer> layers) {
//                    constexpr int kLayerCount = 2;
//                    while (layers.size() < kLayerCount) {
//                        layers.insert(layers.begin(), DensityProfileLayer());
//                    }
//                    std::string result = "DensityProfile(DensityProfileLayer[" +
//                                         std::to_string(kLayerCount) + "](";
//                    for (int i = 0; i < kLayerCount; ++i) {
//                        result += density_layer(layers[i]);
//                        result += i < kLayerCount - 1 ? "," : "))";
//                    }
//                    return result;
//                };
//
//        bool precompute_illuminance = num_precomputed_wavelengths > 3;
//        double sky_k_r, sky_k_g, sky_k_b;
//        if (precompute_illuminance) {
//            sky_k_r = sky_k_g = sky_k_b = MAX_LUMINOUS_EFFICACY;
//        } else {
//            ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance,
//                                                      -3 /* lambda_power */, &sky_k_r, &sky_k_g, &sky_k_b);
//        }
//
//        double sun_k_r, sun_k_g, sun_k_b;
//        ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance,
//                                                  0 /* lambda_power */, &sun_k_r, &sun_k_g, &sun_k_b);
//
//    }
//
///*
//<p>The destructor is trivial:
//*/
//
//    Model::~Model() {
////        glDeleteBuffers(1, &full_screen_quad_vbo_);
////        glDeleteVertexArrays(1, &full_screen_quad_vao_);
////        glDeleteTextures(1, &transmittance_texture_);
////        glDeleteTextures(1, &scattering_texture_);
////        if (optional_single_mie_scattering_texture_ != 0) {
////            glDeleteTextures(1, &optional_single_mie_scattering_texture_);
////        }
////        glDeleteTextures(1, &irradiance_texture_);
////        glDeleteShader(atmosphere_shader_);
//    }
//
//    void Model::Init(unsigned int num_scattering_orders) {
//        vot::Image_sptr deltaIrradianceTex;
//        GLuint delta_irradiance_texture = NewTexture2d(
//                IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
//        GLuint delta_rayleigh_scattering_texture = NewTexture3d(
//                SCATTERING_TEXTURE_WIDTH,
//                SCATTERING_TEXTURE_HEIGHT,
//                SCATTERING_TEXTURE_DEPTH,
//                rgb_format_supported_ ? GL_RGB : GL_RGBA,
//                half_precision_);
//        GLuint delta_mie_scattering_texture = NewTexture3d(
//                SCATTERING_TEXTURE_WIDTH,
//                SCATTERING_TEXTURE_HEIGHT,
//                SCATTERING_TEXTURE_DEPTH,
//                rgb_format_supported_ ? GL_RGB : GL_RGBA,
//                half_precision_);
//        GLuint delta_scattering_density_texture = NewTexture3d(
//                SCATTERING_TEXTURE_WIDTH,
//                SCATTERING_TEXTURE_HEIGHT,
//                SCATTERING_TEXTURE_DEPTH,
//                rgb_format_supported_ ? GL_RGB : GL_RGBA,
//                half_precision_);
//        // delta_multiple_scattering_texture is only needed to compute scattering
//        // order 3 or more, while delta_rayleigh_scattering_texture and
//        // delta_mie_scattering_texture are only needed to compute double scattering.
//        // Therefore, to save memory, we can store delta_rayleigh_scattering_texture
//        // and delta_multiple_scattering_texture in the same GPU texture.
//        GLuint delta_multiple_scattering_texture = delta_rayleigh_scattering_texture;
//
//        // The precomputations also require a temporary framebuffer object, created
//        // here (and destroyed at the end of this method).
////        GLuint fbo;
////        glGenFramebuffers(1, &fbo);
////        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//
//        // The actual precomputations depend on whether we want to store precomputed
//        // irradiance or illuminance values.
//        if (num_precomputed_wavelengths_ <= 3) {
//            vec3 lambdas{kLambdaR, kLambdaG, kLambdaB};
//            mat3 luminance_from_radiance{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
//            Precompute(fbo, delta_irradiance_texture, delta_rayleigh_scattering_texture,
//                       delta_mie_scattering_texture, delta_scattering_density_texture,
//                       delta_multiple_scattering_texture, lambdas, luminance_from_radiance,
//                       false /* blend */, num_scattering_orders);
//        } else {
//            constexpr double kLambdaMin = 360.0;
//            constexpr double kLambdaMax = 830.0;
//            int num_iterations = (num_precomputed_wavelengths_ + 2) / 3;
//            double dlambda = (kLambdaMax - kLambdaMin) / (3 * num_iterations);
//            for (int i = 0; i < num_iterations; ++i) {
//                vec3 lambdas{
//                        kLambdaMin + (3 * i + 0.5) * dlambda,
//                        kLambdaMin + (3 * i + 1.5) * dlambda,
//                        kLambdaMin + (3 * i + 2.5) * dlambda
//                };
//                auto coeff = [dlambda](double lambda, int component) {
//                    // Note that we don't include MAX_LUMINOUS_EFFICACY here, to avoid
//                    // artefacts due to too large values when using half precision on GPU.
//                    // We add this term back in kAtmosphereShader, via
//                    // SKY_SPECTRAL_RADIANCE_TO_LUMINANCE (see also the comments in the
//                    // Model constructor).
//                    double x = CieColorMatchingFunctionTableValue(lambda, 1);
//                    double y = CieColorMatchingFunctionTableValue(lambda, 2);
//                    double z = CieColorMatchingFunctionTableValue(lambda, 3);
//                    return static_cast<float>((
//                                                      XYZ_TO_SRGB[component * 3] * x +
//                                                      XYZ_TO_SRGB[component * 3 + 1] * y +
//                                                      XYZ_TO_SRGB[component * 3 + 2] * z) * dlambda);
//                };
//                mat3 luminance_from_radiance{
//                        coeff(lambdas[0], 0), coeff(lambdas[1], 0), coeff(lambdas[2], 0),
//                        coeff(lambdas[0], 1), coeff(lambdas[1], 1), coeff(lambdas[2], 1),
//                        coeff(lambdas[0], 2), coeff(lambdas[1], 2), coeff(lambdas[2], 2)
//                };
//                Precompute(fbo, delta_irradiance_texture,
//                           delta_rayleigh_scattering_texture, delta_mie_scattering_texture,
//                           delta_scattering_density_texture, delta_multiple_scattering_texture,
//                           lambdas, luminance_from_radiance, i > 0 /* blend */,
//                           num_scattering_orders);
//            }
//
//            // After the above iterations, the transmittance texture contains the
//            // transmittance for the 3 wavelengths used at the last iteration. But we
//            // want the transmittance at kLambdaR, kLambdaG, kLambdaB instead, so we
//            // must recompute it here for these 3 wavelengths:
//            std::string header = glsl_header_factory_({kLambdaR, kLambdaG, kLambdaB});
//            Program compute_transmittance(
//                    kVertexShader, header + kComputeTransmittanceShader);
//            glFramebufferTexture(
//                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, transmittance_texture_, 0);
//            glDrawBuffer(GL_COLOR_ATTACHMENT0);
//            glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
//            compute_transmittance.Use();
//            DrawQuad({}, full_screen_quad_vao_);
//        }
//
//        // Delete the temporary resources allocated at the begining of this method.
//        glUseProgram(0);
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        glDeleteFramebuffers(1, &fbo);
//        glDeleteTextures(1, &delta_scattering_density_texture);
//        glDeleteTextures(1, &delta_mie_scattering_texture);
//        glDeleteTextures(1, &delta_rayleigh_scattering_texture);
//        glDeleteTextures(1, &delta_irradiance_texture);
//        assert(glGetError() == 0);
//    }
//
///*
//<p>The <code>SetProgramUniforms</code> method is straightforward: it simply
//binds the precomputed textures to the specified texture units, and then sets
//the corresponding uniforms in the user provided program to the index of these
//texture units.
//*/
//
//    void Model::SetProgramUniforms(
//            GLuint program,
//            GLuint transmittance_texture_unit,
//            GLuint scattering_texture_unit,
//            GLuint irradiance_texture_unit,
//            GLuint single_mie_scattering_texture_unit) const {
//        glActiveTexture(GL_TEXTURE0 + transmittance_texture_unit);
//        glBindTexture(GL_TEXTURE_2D, transmittance_texture_);
//        glUniform1i(glGetUniformLocation(program, "transmittance_texture"),
//                    transmittance_texture_unit);
//
//        glActiveTexture(GL_TEXTURE0 + scattering_texture_unit);
//        glBindTexture(GL_TEXTURE_3D, scattering_texture_);
//        glUniform1i(glGetUniformLocation(program, "scattering_texture"),
//                    scattering_texture_unit);
//
//        glActiveTexture(GL_TEXTURE0 + irradiance_texture_unit);
//        glBindTexture(GL_TEXTURE_2D, irradiance_texture_);
//        glUniform1i(glGetUniformLocation(program, "irradiance_texture"),
//                    irradiance_texture_unit);
//
//        if (optional_single_mie_scattering_texture_ != 0) {
//            glActiveTexture(GL_TEXTURE0 + single_mie_scattering_texture_unit);
//            glBindTexture(GL_TEXTURE_3D, optional_single_mie_scattering_texture_);
//            glUniform1i(glGetUniformLocation(program, "single_mie_scattering_texture"),
//                        single_mie_scattering_texture_unit);
//        }
//    }
//
///*
//<p>The utility method <code>ConvertSpectrumToLinearSrgb</code> is implemented
//with a simple numerical integration of the given function, times the CIE color
//matching funtions (with an integration step of 1nm), followed by a matrix
//multiplication:
//*/
//
//    void Model::ConvertSpectrumToLinearSrgb(
//            const std::vector<double>& wavelengths,
//            const std::vector<double>& spectrum,
//            double* r, double* g, double* b) {
//        double x = 0.0;
//        double y = 0.0;
//        double z = 0.0;
//        const int dlambda = 1;
//        for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
//            double value = Interpolate(wavelengths, spectrum, lambda);
//            x += CieColorMatchingFunctionTableValue(lambda, 1) * value;
//            y += CieColorMatchingFunctionTableValue(lambda, 2) * value;
//            z += CieColorMatchingFunctionTableValue(lambda, 3) * value;
//        }
//        *r = MAX_LUMINOUS_EFFICACY *
//             (XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
//        *g = MAX_LUMINOUS_EFFICACY *
//             (XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
//        *b = MAX_LUMINOUS_EFFICACY *
//             (XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
//    }
//
///*
//<p>Finally, we provide the actual implementation of the precomputation algorithm
//described in Algorithm 4.1 of
//<a href="https://hal.inria.fr/inria-00288758/en">our paper</a>. Each step is
//explained by the inline comments below.
//*/
//    void Model::Precompute(
//            GLuint fbo,
//            GLuint delta_irradiance_texture,
//            GLuint delta_rayleigh_scattering_texture,
//            GLuint delta_mie_scattering_texture,
//            GLuint delta_scattering_density_texture,
//            GLuint delta_multiple_scattering_texture,
//            const vec3& lambdas,
//            const mat3& luminance_from_radiance,
//            bool blend,
//            unsigned int num_scattering_orders) {
//        // The precomputations require specific GLSL programs, for each precomputation
//        // step. We create and compile them here (they are automatically destroyed
//        // when this method returns, via the Program destructor).
//        std::string header = glsl_header_factory_(lambdas);
//        Program compute_transmittance(
//                kVertexShader, header + kComputeTransmittanceShader);
//        Program compute_direct_irradiance(
//                kVertexShader, header + kComputeDirectIrradianceShader);
//        Program compute_single_scattering(kVertexShader, kGeometryShader,
//                                          header + kComputeSingleScatteringShader);
//        Program compute_scattering_density(kVertexShader, kGeometryShader,
//                                           header + kComputeScatteringDensityShader);
//        Program compute_indirect_irradiance(
//                kVertexShader, header + kComputeIndirectIrradianceShader);
//        Program compute_multiple_scattering(kVertexShader, kGeometryShader,
//                                            header + kComputeMultipleScatteringShader);
//
//        const GLuint kDrawBuffers[4] = {
//                GL_COLOR_ATTACHMENT0,
//                GL_COLOR_ATTACHMENT1,
//                GL_COLOR_ATTACHMENT2,
//                GL_COLOR_ATTACHMENT3
//        };
//        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
//        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
//
//        // Compute the transmittance, and store it in transmittance_texture_.
//        glFramebufferTexture(
//                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, transmittance_texture_, 0);
//        glDrawBuffer(GL_COLOR_ATTACHMENT0);

//        glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
//        compute_transmittance.Use();
//        DrawQuad({}, full_screen_quad_vao_);
//
//        // Compute the direct irradiance, store it in delta_irradiance_texture and,
//        // depending on 'blend', either initialize irradiance_texture_ with zeros or
//        // leave it unchanged (we don't want the direct irradiance in
//        // irradiance_texture_, but only the irradiance from the sky).
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//                             delta_irradiance_texture, 0);
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
//                             irradiance_texture_, 0);
//        glDrawBuffers(2, kDrawBuffers);
//        glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
//        compute_direct_irradiance.Use();
//        compute_direct_irradiance.BindTexture2d(
//                "transmittance_texture", transmittance_texture_, 0);
//        DrawQuad({false, blend}, full_screen_quad_vao_);
//
//        // Compute the rayleigh and mie single scattering, store them in
//        // delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
//        // either store them or accumulate them in scattering_texture_ and
//        // optional_single_mie_scattering_texture_.
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//                             delta_rayleigh_scattering_texture, 0);
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
//                             delta_mie_scattering_texture, 0);
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
//                             scattering_texture_, 0);
//        if (optional_single_mie_scattering_texture_ != 0) {
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,
//                                 optional_single_mie_scattering_texture_, 0);
//            glDrawBuffers(4, kDrawBuffers);
//        } else {
//            glDrawBuffers(3, kDrawBuffers);
//        }
//        glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
//        compute_single_scattering.Use();
//        compute_single_scattering.BindMat3(
//                "luminance_from_radiance", luminance_from_radiance);
//        compute_single_scattering.BindTexture2d(
//                "transmittance_texture", transmittance_texture_, 0);
//        for (unsigned int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
//            compute_single_scattering.BindInt("layer", layer);
//            DrawQuad({false, false, blend, blend}, full_screen_quad_vao_);
//        }
//
//        // Compute the 2nd, 3rd and 4th order of scattering, in sequence.
//        for (unsigned int scattering_order = 2;
//             scattering_order <= num_scattering_orders;
//             ++scattering_order) {
//            // Compute the scattering density, and store it in
//            // delta_scattering_density_texture.
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//                                 delta_scattering_density_texture, 0);
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
//            glDrawBuffer(GL_COLOR_ATTACHMENT0);
//            glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
//            compute_scattering_density.Use();
//            compute_scattering_density.BindTexture2d(
//                    "transmittance_texture", transmittance_texture_, 0);
//            compute_scattering_density.BindTexture3d(
//                    "single_rayleigh_scattering_texture",
//                    delta_rayleigh_scattering_texture,
//                    1);
//            compute_scattering_density.BindTexture3d(
//                    "single_mie_scattering_texture", delta_mie_scattering_texture, 2);
//            compute_scattering_density.BindTexture3d(
//                    "multiple_scattering_texture", delta_multiple_scattering_texture, 3);
//            compute_scattering_density.BindTexture2d(
//                    "irradiance_texture", delta_irradiance_texture, 4);
//            compute_scattering_density.BindInt("scattering_order", scattering_order);
//            for (unsigned int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
//                compute_scattering_density.BindInt("layer", layer);
//                DrawQuad({}, full_screen_quad_vao_);
//            }
//
//            // Compute the indirect irradiance, store it in delta_irradiance_texture and
//            // accumulate it in irradiance_texture_.
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//                                 delta_irradiance_texture, 0);
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
//                                 irradiance_texture_, 0);
//            glDrawBuffers(2, kDrawBuffers);
//            glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
//            compute_indirect_irradiance.Use();
//            compute_indirect_irradiance.BindMat3(
//                    "luminance_from_radiance", luminance_from_radiance);
//            compute_indirect_irradiance.BindTexture3d(
//                    "single_rayleigh_scattering_texture",
//                    delta_rayleigh_scattering_texture,
//                    0);
//            compute_indirect_irradiance.BindTexture3d(
//                    "single_mie_scattering_texture", delta_mie_scattering_texture, 1);
//            compute_indirect_irradiance.BindTexture3d(
//                    "multiple_scattering_texture", delta_multiple_scattering_texture, 2);
//            compute_indirect_irradiance.BindInt("scattering_order",
//                                                scattering_order - 1);
//            DrawQuad({false, true}, full_screen_quad_vao_);
//
//            // Compute the multiple scattering, store it in
//            // delta_multiple_scattering_texture, and accumulate it in
//            // scattering_texture_.
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//                                 delta_multiple_scattering_texture, 0);
//            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
//                                 scattering_texture_, 0);
//            glDrawBuffers(2, kDrawBuffers);
//            glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
//            compute_multiple_scattering.Use();
//            compute_multiple_scattering.BindMat3(
//                    "luminance_from_radiance", luminance_from_radiance);
//            compute_multiple_scattering.BindTexture2d(
//                    "transmittance_texture", transmittance_texture_, 0);
//            compute_multiple_scattering.BindTexture3d(
//                    "scattering_density_texture", delta_scattering_density_texture, 1);
//            for (unsigned int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
//                compute_multiple_scattering.BindInt("layer", layer);
//                DrawQuad({false, true}, full_screen_quad_vao_);
//            }
//        }
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
//        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
//    }
//
//}  // namespace atmosphere