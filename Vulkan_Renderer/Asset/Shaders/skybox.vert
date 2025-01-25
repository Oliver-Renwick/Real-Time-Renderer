#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 UVW;

void main()
{
    UVW = inPosition;

    mat4 viewMat = mat4(mat3(ubo.view));
    gl_Position = ubo.proj * viewMat * vec4(inPosition.xyz, 1.0);
}