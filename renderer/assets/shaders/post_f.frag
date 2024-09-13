//#version 460
//layout(location = 0) in vec2 outUV;
//layout(location = 0) out vec4 fragColor;
//
//layout(set = 0, binding = 0) uniform sampler2D raytracing;
//layout(set = 0, binding = 1) uniform sampler2D noisyTxt;
//
//void main()
//{
//    vec2  uv    = outUV;
////    float gamma = 1.f / 2.2f;
////    fragColor   = pow(texture(noisyTxt, uv).rgba, vec4(gamma));
//
//    vec4 rt = texture(raytracing, uv).rgba;
//    vec4 color = texture(noisyTxt, uv).rgba;
//
//    fragColor = color * rt;
//}


#version 460
layout(location = 0) in vec2 outUV;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D noisyTxt;
layout(set = 0, binding = 1) uniform sampler2D test;

void main()
{
    vec2  uv    = outUV;
    //    float gamma = 1.f / 2.2f;
    //    fragColor   = pow(texture(noisyTxt, uv).rgba, vec4(gamma));

    fragColor = texture(noisyTxt, uv).rgba;
}
