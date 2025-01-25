#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
    vec4 ViewPos;
} ubo;


layout(push_constant) uniform PushConsts {
	mat4 model;
} primitive;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inColor;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outViewVec;
layout(location = 4) out vec3 outLightVec;
layout(location = 5) out vec4 outTangent;

void main() {
    
    outNormal = inNormal;
    outUV = inUV;
    outTangent = inTangent;
    outColor = inColor;
    gl_Position = ubo.proj * ubo.view * primitive.model * vec4(inPosition, 1.0);

    outNormal = mat3(primitive.model) * inNormal;
    vec4 pos = primitive.model * vec4(inPosition, 1.0);
    outLightVec = ubo.lightPos.xyz - pos.xyz;
    outViewVec = ubo.ViewPos.xyz - pos.xyz;
}