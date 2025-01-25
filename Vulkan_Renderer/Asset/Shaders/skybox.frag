#version 450

layout (set = 1, binding = 2) uniform samplerCube samplerCubeMap;

layout(location = 0) in vec3 UVW;

layout(location = 0) out vec4 outFrag;


void main()
{
	outFrag = texture(samplerCubeMap, UVW);
}
