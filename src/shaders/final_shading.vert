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
layout(location = 3) out vec3 fragPosWorldSpace;
layout(location = 4) out vec3 fragPosViewSpace;
layout(location = 5) out vec3 cameraPosWorldSpace;
layout(location = 6) out mat3 TBN;


out gl_PerVertex {
    vec4 gl_Position;
};

mat3 getTBN(vec3 geomnor)
{
    vec3 up = vec3(0.001, 1.0, 0.001);
    vec3 tangent = normalize(cross(geomnor, up));
    vec3 bitangent = cross(geomnor, tangent);
    return mat3(tangent, bitangent, geomnor);
}

void main() {
   
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;

    vec4 posWorldSpace = ubo.model * vec4(inPosition, 1.0);
    fragPosWorldSpace = posWorldSpace.xyz / posWorldSpace.w;

    vec4 posViewSpace = ubo.view * posWorldSpace;
    fragPosViewSpace = posViewSpace.xyz / posViewSpace.w;

    vec4 posProjSpace = ubo.proj * posViewSpace;
    gl_Position = posProjSpace;

    cameraPosWorldSpace = ubo.cameraPos.xyz;

    TBN = getTBN(inNormal);
}