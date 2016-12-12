#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define MAX_NUM_LIGHTS_PER_TILE 1024
#define PIXELS_PER_TILE 8

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

layout(binding = 6) uniform Params {
	int numLights;
	float time;
    int debugMode;
    // int padding;
	ivec2 numThreads;
    ivec2 screenDimensions;
} params;

layout(binding = 5) buffer Frustums {
    Frustum frustums[];
};

layout(binding = 3) buffer Lights {
	Light lights[];
};

layout(binding = 7) buffer LightIndex {
	int lightIndices[];
};

layout(binding = 8) buffer LightGrid {
	int lightGrid[];
};

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
    vec3 up = normalize(vec3(0.00001, 1, 0.00001));
    vec3 surftan = normalize(cross(geomnor, up));
    vec3 surfbinor = cross(geomnor, surftan);
    vec3 mappedNor = normap.y * surftan + normap.x * surfbinor + normap.z * geomnor;
    return normalize(mappedNor);
}

void main() {

	ivec2 tileID = ivec2(gl_FragCoord.x, params.screenDimensions.y - gl_FragCoord.y ) / PIXELS_PER_TILE;
	int index = tileID.y * params.numThreads.x + tileID.x;
    vec3 finalColor = vec3(0,0,0);
    // lighting for each light
    vec3 lightPos, lightDir, lightColor;
    vec3 viewDir = normalize(cameraPosWorldSpace.xyz - fragPosWorldSpace);
    float dist, lightIntensity, NdotL, lightRadius;

    vec3 textureColor = texture(texColorSampler, fragTexCoord).xyz;
    vec3 normalMap = texture(texNormalSampler, fragTexCoord).xyz ;
    //vec3 normal = applyNormalMap(fragNormal, normalMap);
    vec3 normal = fragNormal;

    uint lightIndexBegin = index * MAX_NUM_LIGHTS_PER_TILE;
    uint lightNum = lightGrid[index];

    // lightGrid[index] = lights need to be considered in tile
    for(int i = 0; i < lightNum; ++i) {
        int lightIndex = lightIndices[i + lightIndexBegin];

        Light currentLight = lights[lightIndex];

		vec3 beginPos = currentLight.beginPos.xyz;
		vec3 endPos = currentLight.endPos.xyz;
		float t = sin(params.time * lightIndex * .0005f);

        lightPos = (1 - t) * beginPos + t * endPos;
        lightColor = currentLight.color.xyz;
        lightDir = lightPos - fragPosWorldSpace;
        lightIntensity = currentLight.beginPos.w;
        lightRadius = currentLight.endPos.w;

        dist = length(lightDir);
        lightDir = lightDir/dist;

        NdotL = max(0, dot(normal, lightDir));

        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        float specular = pow(specAngle, 32.0);

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

        case 2: // normal with normal mapping
        outColor = vec4(normal, 1.0);
            break;

        case 3: // distance to camera in view space
        float dist = 1.0 / distance(fragPosWorldSpace, cameraPosWorldSpace);
        outColor = vec4(dist * vec3(1,1,1), 1.0);
            break;

        case 4: // position in world space
        outColor = vec4(fragPosWorldSpace, 1.0);
            break;

        case 5: // normal map
        outColor = vec4(abs(normalMap), 1.0);
            break;

		// case 6: // fragment id map
  //       outColor = vec4(tileID / vec2(params.numThreads.xy), 0.f, 1.f);
  //       	break;

		case 6: // light heat map
        float tmp = lightGrid[index];
        if(tmp <= 100.f)
        {
            outColor = vec4( 0.f, 0.f, tmp / 100.f, 1.f );
        }
        else if(tmp > 100.f && tmp <= 200.f) {
            outColor = vec4( 0.f, (tmp - 100.f) / 100.0f, 1.f, 1.f );
        }
        else {
            outColor = vec4( (tmp - 200.f) / 100.f, 1.f, 1.f, 1.f );
        }
			break;

        default:
        outColor = vec4(finalColor, 1.0);
            break;

    }
}
