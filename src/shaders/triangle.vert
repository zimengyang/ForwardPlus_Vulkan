#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPosWorldSapce;
layout(location = 4) out float depth;
layout(location = 5) out vec4 cameraPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
   
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;

    vec4 posWorldSpace = ubo.model * vec4(inPosition, 1.0);
    fragPosWorldSapce = posWorldSpace.xyz / posWorldSpace.w;

    vec4 posViewSpace = ubo.view * posWorldSpace;
    depth = posViewSpace.z / posViewSpace.w;

    vec4 posProjSpace = ubo.proj * posViewSpace;
    gl_Position = posProjSpace;

    cameraPos = ubo.cameraPos;
}