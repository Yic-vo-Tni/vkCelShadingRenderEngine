#ifndef ATMOSPHERE_MODEL_H_
#define ATMOSPHERE_MODEL_H_

//#include <glad/glad.h>
#include <array>
#include <functional>
#include <string>
#include <vector>
#include "RHI/Image.h"

namespace atmosphere {

    class DensityProfileLayer {
    public:
        DensityProfileLayer() : DensityProfileLayer(0.0, 0.0, 0.0, 0.0, 0.0) {}
        DensityProfileLayer(double width, double exp_term, double exp_scale,
                            double linear_term, double constant_term)
                : width(width), exp_term(exp_term), exp_scale(exp_scale),
                  linear_term(linear_term), constant_term(constant_term) {
        }
        double width;
        double exp_term;
        double exp_scale;
        double linear_term;
        double constant_term;
    };

    class Model {
    public:
        Model(
                const std::vector<double>& wavelengths,
                const std::vector<double>& solar_irradiance,
                double sun_angular_radius,
                double bottom_radius,
                double top_radius,
                const std::vector<DensityProfileLayer>& rayleigh_density,
                const std::vector<double>& rayleigh_scattering,
                const std::vector<DensityProfileLayer>& mie_density,
                const std::vector<double>& mie_scattering,
                const std::vector<double>& mie_extinction,
                double mie_phase_function_g,
                const std::vector<DensityProfileLayer>& absorption_density,
                const std::vector<double>& absorption_extinction,
                const std::vector<double>& ground_albedo,
                double max_sun_zenith_angle,
                double length_unit_in_meters,
                unsigned int num_precomputed_wavelengths,
                bool combine_scattering_textures,
                bool half_precision);

        ~Model();

        void Init(unsigned int num_scattering_orders = 4);

//        GLuint shader() const { return atmosphere_shader_; }
//
//        void SetProgramUniforms(
//                GLuint program,
//                GLuint transmittance_texture_unit,
//                GLuint scattering_texture_unit,
//                GLuint irradiance_texture_unit,
//                GLuint optional_single_mie_scattering_texture_unit = 0) const;

        // Utility method to convert a function of the wavelength to linear sRGB.
        // 'wavelengths' and 'spectrum' must have the same size. The integral of
        // 'spectrum' times each CIE_2_DEG_COLOR_MATCHING_FUNCTIONS (and times
        // MAX_LUMINOUS_EFFICACY) is computed to get XYZ values, which are then
        // converted to linear sRGB with the XYZ_TO_SRGB matrix.
        static void ConvertSpectrumToLinearSrgb(
                const std::vector<double>& wavelengths,
                const std::vector<double>& spectrum,
                double* r, double* g, double* b);

        static constexpr double kLambdaR = 680.0;
        static constexpr double kLambdaG = 550.0;
        static constexpr double kLambdaB = 440.0;

    private:
        typedef std::array<double, 3> vec3;
        typedef std::array<float, 9> mat3;

//        void Precompute(
//                GLuint fbo,
//                GLuint delta_irradiance_texture,
//                GLuint delta_rayleigh_scattering_texture,
//                GLuint delta_mie_scattering_texture,
//                GLuint delta_scattering_density_texture,
//                GLuint delta_multiple_scattering_texture,
//                const vec3& lambdas,
//                const mat3& luminance_from_radiance,
//                bool blend,
//                unsigned int num_scattering_orders);

        unsigned int num_precomputed_wavelengths_;
        bool half_precision_;
//        bool rgb_format_supported_;
//        std::function<std::string(const vec3&)> glsl_header_factory_;
//        GLuint transmittance_texture_;
//        GLuint scattering_texture_;
//        GLuint optional_single_mie_scattering_texture_;
//        GLuint irradiance_texture_;
//        GLuint atmosphere_shader_;
//        GLuint full_screen_quad_vao_;
//        GLuint full_screen_quad_vbo_;
        vot::Image_sptr transmittanceTex;
        vot::Image_sptr scatteringTex;
        vot::Image_sptr singleMieScatteringTex;
        vot::Image_sptr irradianceTex;
    };

}  // namespace atmosphere

#endif  // ATMOSPHERE_MODEL_H_