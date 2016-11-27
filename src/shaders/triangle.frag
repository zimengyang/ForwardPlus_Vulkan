#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in float depth;
layout(location = 4) in float debugMode;

layout(location = 0) out vec4 outColor;


void main() {
    
    //outColor = vec4(abs(depth) * vec3(1,1,1), 1);
    outColor = vec4(normal, 1.0);
        return;

    // if(debugMode == 0) // texture color
    // {    
    //     outColor = texture(texSampler, fragTexCoord);
    //     return;
    // }
    // else if(debugMode == 1) // tex coord debug
    // {
    //     outColor = vec4(fragTexCoord, 0.0, 1.0);
    //     return;
    // }
    // else if(debugMode == 2) // normal 
    // {
    //     outColor = vec4(normal, 1.0);
    //     return;
    // }
    // else if(debugMode == 3) // depth 
    // {
    //     outColor = vec4(abs(depth) * vec3(1,1,1), 1);
    //     return;
    // }
    // else 
    // {
    //     outColor = texture(texSampler, fragTexCoord);
    //     return;
    // }

}