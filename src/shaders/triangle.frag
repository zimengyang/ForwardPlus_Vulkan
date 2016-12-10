#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Light {
	vec4 beginPos; // beginPos.w is intensity
	vec4 endPos; // endPos.w is radius
	vec4 color; // color.w is t
};

struct Frustum {
    vec4 planes[4];
};

layout(binding = 1) uniform sampler2D texColorSampler;
layout(binding = 2) uniform sampler2D texNormalSampler;

layout(binding = 3) buffer Lights {
	Light lights[];
};

layout(binding = 5) buffer Frustums {
    Frustum frustums[];
};

layout(binding = 6) uniform Params {
	int numLights;
	float time;
    int debugMode;
} params;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosWorldSpace;
layout(location = 4) in vec3 fragPosViewSpace;
layout(location = 5) in vec3 cameraPosWorldSpace;

layout(location = 0) out vec4 outColor;

// apply normal map
vec3 applyNormalMap(vec3 geomnor, vec3 normap) {
    normap = normap * 2.0 - 1.0;
    vec3 up = normalize(vec3(0.001, 1, 0.001));
    vec3 surftan = normalize(cross(geomnor, up));
    vec3 surfbinor = cross(geomnor, surftan);
    return normap.y * surftan + normap.x * surfbinor + normap.z * geomnor;
}

void main() {

    vec3 finalColor = vec3(0,0,0);
    // lighting for each light
    vec3 lightPos, lightDir, lightColor;
    vec3 viewDir = normalize(cameraPosWorldSpace.xyz - fragPosWorldSpace);
    float dist, lightIntensity, NdotL, lightRadius;

    vec3 textureColor = texture(texColorSampler, fragTexCoord).xyz;
    vec3 normalMap = texture(texNormalSampler, fragTexCoord).xyz ;
    //vec3 normal = applyNormalMap(fragNormal, normalMap);
    vec3 normal = fragNormal;
    //vec3 normal = normalMap;

    for(int i = 0; i < params.numLights; ++i) {
		vec3 beginPos = lights[i].beginPos.xyz;
		vec3 endPos = lights[i].endPos.xyz;
		float t = sin(params.time * 0.2);

        lightPos = (1 - t) * beginPos + t * endPos;
        lightColor = lights[i].color.xyz;
        lightDir = lightPos - fragPosWorldSpace;
        lightIntensity = lights[i].beginPos.w;
        lightRadius = lights[i].endPos.w;

        dist = length(lightDir);
        lightDir = lightDir/dist;

        // attenuation
        //float att = max(0, lightRadius - dist);

        NdotL = dot(normal, lightDir);

        //finalColor += max(0, NdotL) * lightColor * att * lightIntensity;
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        float specular = pow(specAngle, 16.0);

        vec3 tmpColor = (0.8 * NdotL + 0.2 * specular) * lightColor * lightIntensity;

        float att = max(0.0, lightRadius - dist);
        finalColor += att * tmpColor;

    }

    finalColor = finalColor * textureColor;
    outColor = vec4(finalColor, 1.0);

    switch(params.debugMode){
        case 0: // lighting 
        outColor = vec4(finalColor, 1.0);
            break;

        case 1: // texture map
        outColor = vec4(textureColor, 1.0);
            break;

        case 2: // distance to camera in view space
        float dist = 1.0 / distance(fragPosWorldSpace, cameraPosWorldSpace);
        outColor = vec4(dist * vec3(1,1,1), 1.0);
            break;

        default:
        outColor = vec4(finalColor, 1.0);
            break;

    }
}
