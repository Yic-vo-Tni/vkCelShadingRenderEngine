#version 460
layout(location = 0) in vec2 outUV;
layout(location = 0) out vec4 fragColor;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inputColor;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inputVolumetric;
layout(set = 1, binding = 2) uniform sampler2D raytracing;

void main()
{
    vec2  uv    = outUV;
//    float gamma = 1.f / 2.2f;
//    fragColor   = pow(texture(noisyTxt, uv).rgba, vec4(gamma));

    vec4 rt = texture(raytracing, uv).rgba;
    vec4 volumetric = subpassLoad(inputVolumetric);
    vec4 color = subpassLoad(inputColor);

    if(color.a > 0.99f){
        fragColor = color * rt;
    } else {
        fragColor = volumetric;
    }

//    fragColor = color * rt;
}


/// test

//#version 460
//layout(location = 0) in vec2 outUV;
//layout(location = 0) out vec4 fragColor;
//
//layout(set = 0, binding = 0) uniform sampler2D noisyTxt;
//layout(set = 0, binding = 1) uniform sampler2D test;
//
//void main()
//{
//    vec2  uv    = outUV;
//    //    float gamma = 1.f / 2.2f;
//    //    fragColor   = pow(texture(noisyTxt, uv).rgba, vec4(gamma));
//
//    fragColor = texture(noisyTxt, uv).rgba;
//}
