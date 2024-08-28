#version 450

layout (location = 0) in vec2 fragUv;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform sampler2D texSampler;

void main() {
    vec4 diff = texture(texSampler, fragUv).rgba;

	if(diff.a < 1.f){
		discard;
} else{
	outColor = diff;
}
}
