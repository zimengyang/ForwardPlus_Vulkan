#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define NUM_LIGHTS 8

struct LightStruct {
	vec4 beginPos; // beginPos.w is intensity
	vec4 endPos; // endPos.w is radius
	vec4 color; // color.w is t
};

struct Frustum
{
    vec4 planes[4];
};

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 3) buffer Lights {
	LightStruct lights[];
};

layout(binding = 5) buffer FrustumGrid {
   Frustum frustums[];
   int numFrustums;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 fragPosWorldSpace;
layout(location = 4) in float depth;
layout(location = 5) in vec4 cameraPos;

layout(location = 0) out vec4 outColor;

void main() {

    //float depth = fragPosWorldSpace.z;
    //outColor = vec4(abs(depth) * vec3(1,1,1), 1); // depth
    //outColor = vec4(normal, 1.0); // normal

    vec3 finalColor = vec3(0,0,0);
    // lighting for each light
    vec3 lightPos, lightDir, lightColor;
    vec3 viewDir = normalize(cameraPos.xyz - fragPosWorldSpace);
    float dist, lightIntensity, NdotL, lightRadius;

    for(int i = 0; i < NUM_LIGHTS; ++i) {
		vec3 beginPos = lights[i].beginPos.xyz;
		vec3 endPos = lights[i].endPos.xyz;
		float t = sin(lights[i].color.w);

        lightPos = (1 - t) * beginPos + t * endPos;
        lightColor = frustums[0].planes[0].xyz; //lights[i].color.xyz;
        lightDir = lightPos - fragPosWorldSpace;
        lightIntensity = lights[i].beginPos.w;
        lightRadius = lights[i].endPos.w;

        dist = length(lightDir);
        lightDir = lightDir/dist;

        // attenuation
        //float att = max(0, lightRadius - dist);

        NdotL = dot(normal,lightDir);

        //finalColor += max(0, NdotL) * lightColor * att * lightIntensity;
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        float specular = pow(specAngle, 16.0);

        vec3 tmpColor = (0.8 * NdotL + 0.2 * specular) * lightColor * lightIntensity;

        float att = max(0.0, lightRadius - dist);
        finalColor += att * tmpColor;

    }
    //finalColor = finalColor * texture(texSampler, fragTexCoord).xyz;
    outColor = vec4(finalColor, 1.0);
}
