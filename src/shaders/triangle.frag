#version 450
#extension GL_ARB_separate_shader_objects : enable

struct LightInfo {
    vec4 pos;
    vec4 color;
};

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 3) uniform LightInfos {
    LightInfo lights[8];
    int numLights;
} lightInfos;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 fragPosWorldSpace;
layout(location = 4) in float depth;

layout(location = 0) out vec4 outColor;


void main() {
    
    //float depth = fragPosWorldSpace.z;
    //outColor = vec4(abs(depth) * vec3(1,1,1), 1); // depth
    //outColor = vec4(normal, 1.0); // normal

    vec3 finalColor = vec3(0,0,0);
    // lighting
    vec3 lightPos, lightDir, lightColor;
    float dist, lightIntensity, NdotL, lightRadius;
    for(int i=0;i<lightInfos.numLights;++i){
        lightPos = lightInfos.lights[i].pos.xyz;
        lightColor = lightInfos.lights[i].color.xyz;
        lightDir = lightPos - fragPosWorldSpace;
        lightIntensity = lightInfos.lights[i].pos.w;
        lightRadius = lightInfos.lights[i].color.w;

        dist = length(lightDir);
        lightDir = lightDir/dist;

        // attenuation
        float att = max(0, lightRadius - dist);

        NdotL = dot(normal,lightDir);
        
        finalColor += max(0, NdotL) * lightColor * att * lightIntensity;
    }
    //finalColor = finalColor * texture(texSampler, fragTexCoord).xyz;
    outColor = vec4(finalColor, 1.0);

    //outColor = vec4(abs(1.0 / depth) * vec3(1,1,1), 1);

    //outColor = vec4(abs(normal), 1.0);

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